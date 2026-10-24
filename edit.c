#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "types.h"
#include "edit.h"

static const char *id22[TAG_COUNT] = {"TT2", "TP1", "TAL", "TYE", "TCO", "COM"};
static uint32_t syncsafe(const unsigned char *p) { return ((uint32_t)p[0]<<21)|((uint32_t)p[1]<<14)|((uint32_t)p[2]<<7)|p[3]; }
static uint32_t be32(const unsigned char *p) { return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3]; }
static void put32(unsigned char *p,uint32_t n) { p[0]=(unsigned char)(n>>24);p[1]=(unsigned char)(n>>16);p[2]=(unsigned char)(n>>8);p[3]=(unsigned char)n; }
static void put_syncsafe(unsigned char *p,uint32_t n) { p[0]=(unsigned char)((n>>21)&127);p[1]=(unsigned char)((n>>14)&127);p[2]=(unsigned char)((n>>7)&127);p[3]=(unsigned char)(n&127); }
static void set_error(char *out,size_t n,const char *m) { if(n)snprintf(out,n,"%s",m); }
static int frame_field(const unsigned char *id,int v22) { int i; const char **ids=v22?id22:TAG_FRAME_IDS; for(i=0;i<TAG_COUNT;++i)if(!memcmp(id,ids[i],v22?3:4))return i; return -1; }

int write_mp3_tag(const char *path, TagField field, const char *value, char *error, size_t error_size)
{
    FILE *in = NULL, *out = NULL;
    unsigned char h[10], *old = NULL, *frames = NULL;
    unsigned char buffer[4096];
    uint32_t oldsize = 0, used = 0;
    size_t cap = 0, pos = 0, hdr = 10, count;
    int ver = 3;
    char temp_path[1024];
    /* Open the MP3 and read its existing ID3 header, if it has one. */
    if(field<0||field>=TAG_COUNT){set_error(error,error_size,"Invalid tag field.");return 0;}
    in=fopen(path,"rb"); if(!in){set_error(error,error_size,"Could not open MP3 file.");return 0;}
    if(fread(h,1,10,in)==10 && !memcmp(h,"ID3",3)) {
        ver=h[3]; if(ver==4){fclose(in);set_error(error,error_size,"Editing ID3v2.4 is not supported.");return 0;}
        if(ver!=2 && ver!=3){fclose(in);set_error(error,error_size,"Unsupported ID3v2 version.");return 0;}
        oldsize=syncsafe(h+6); old=(unsigned char*)malloc(oldsize?oldsize:1);
        if(!old || fread(old,1,oldsize,in)!=oldsize){free(old);fclose(in);set_error(error,error_size,"Could not read existing ID3 tag.");return 0;}
    } else { rewind(in); oldsize=0; }
    if(ver==2)hdr=6;
    cap=(size_t)oldsize+strlen(value)+32; frames=(unsigned char*)malloc(cap); if(!frames){free(old);fclose(in);set_error(error,error_size,"Out of memory.");return 0;}
    /* Copy every existing frame except the one the user asked to change. */
    while(pos+hdr<=oldsize) {
        unsigned char *fr=old+pos; uint32_t n; size_t total;
        if(fr[0]==0) break;
        n=ver==2?((uint32_t)fr[3]<<16| (uint32_t)fr[4]<<8|fr[5]):be32(fr+4); total=hdr+n;
        if(n>oldsize-pos-hdr)break;
        if (frame_field(fr, ver == 2) != (int)field) {
            memcpy(frames+used,fr,total);
            used+=(uint32_t)total;
        }
        pos+=total;
    }
    /* Add a fresh frame containing the updated value. */
    {
        unsigned char frame[10]; size_t n=strlen(value)+1;
        if(field==TAG_COMMENT) n+=4; /* encoding byte, ISO-8859-1 language, empty description */
        if(ver==2){memcpy(frame,id22[field],3);frame[3]=(unsigned char)(n>>16);frame[4]=(unsigned char)(n>>8);frame[5]=(unsigned char)n;}
        else {memcpy(frame,TAG_FRAME_IDS[field],4);put32(frame+4,(uint32_t)n);frame[8]=frame[9]=0;}
        if(used+hdr+n>cap){unsigned char *b=(unsigned char*)realloc(frames,used+hdr+n);if(!b){free(old);free(frames);fclose(in);set_error(error,error_size,"Out of memory.");return 0;}frames=b;cap=used+hdr+n;}
        memcpy(frames+used,frame,hdr); used+=(uint32_t)hdr;
        if(field==TAG_COMMENT) { frames[used++]=0; memcpy(frames+used,"eng\0",4); used+=4; memcpy(frames+used,value,n-5); used+=(uint32_t)(n-5); }
        else { frames[used++]=0; memcpy(frames+used,value,n-1); used+=(uint32_t)(n-1); }
    }
    /* Write the new tag and then copy the original audio data unchanged. */
    if (snprintf(temp_path, sizeof(temp_path), "%s.codex-tmp", path) >= (int)sizeof(temp_path)) {
        free(old);
        free(frames);
        fclose(in);
        set_error(error, error_size, "MP3 path is too long.");
        return 0;
    }
    /* Never overwrite a leftover recovery file from an earlier failed edit. */
    out = fopen(temp_path, "rb");
    if (out != NULL) {
        fclose(out);
        free(old);
        free(frames);
        fclose(in);
        set_error(error, error_size,
                  "Temporary output file already exists; preserve or rename it first.");
        return 0;
    }
    out = fopen(temp_path, "wb");
    if (!out) {
        free(old);
        free(frames);
        fclose(in);
        set_error(error, error_size, "Could not create temporary output file.");
        return 0;
    }
    memset(h,0,10);memcpy(h,"ID3",3);h[3]=(unsigned char)ver;h[4]=0;put_syncsafe(h+6,used);
    if (fwrite(h, 1, 10, out) != 10 || fwrite(frames, 1, used, out) != used) {
        fclose(in);
        fclose(out);
        remove(temp_path);
        free(old);
        free(frames);
        set_error(error, error_size, "Could not write the new ID3 tag.");
        return 0;
    }

    if (oldsize != 0) {
        if (fseek(in, 10L + oldsize, SEEK_SET) != 0) {
            fclose(in);
            fclose(out);
            remove(temp_path);
            free(old);
            free(frames);
            set_error(error, error_size, "Could not seek to the MP3 audio data.");
            return 0;
        }
    } else {
        rewind(in);
    }

    /* Copy audio bytes using a buffer separate from the temporary filename. */
    while ((count = fread(buffer, 1, sizeof(buffer), in)) > 0) {
        if (fwrite(buffer, 1, count, out) != count) {
            fclose(in);
            fclose(out);
            remove(temp_path);
            free(old);
            free(frames);
            set_error(error, error_size, "Could not copy the MP3 audio data.");
            return 0;
        }
    }

    if (ferror(in)) {
        fclose(in);
        fclose(out);
        remove(temp_path);
        free(old);
        free(frames);
        set_error(error, error_size, "Could not read the MP3 audio data.");
        return 0;
    }
    fclose(in);

    if (fclose(out) != 0) {
        remove(temp_path);
        free(old);
        free(frames);
        set_error(error, error_size, "Could not finish the temporary MP3 file.");
        return 0;
    }

    /* rename replaces the destination on POSIX systems; if it fails, the source remains. */
    if (rename(temp_path, path) != 0) {
        free(old);
        free(frames);
        set_error(error, error_size, "Could not replace original MP3 file; it was left unchanged.");
        return 0;
    }
    free(old);free(frames);set_error(error,error_size,"");return 1;
}

