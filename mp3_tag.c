#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "types.h"
#include "view.h"

const char *TAG_NAMES[TAG_COUNT] = {"Title", "Artist", "Album", "Year", "Genre", "Comment"};
const char *TAG_FRAME_IDS[TAG_COUNT] = {"TIT2", "TPE1", "TALB", "TYER", "TCON", "COMM"};

static const char *id22[TAG_COUNT] = {"TT2", "TP1", "TAL", "TYE", "TCO", "COM"};
static const unsigned char id1_genre[] = "Blues\0Classic Rock\0Country\0Dance\0Disco\0Funk\0Grunge\0Hip-Hop\0Jazz\0Metal\0New Age\0Oldies\0Other\0Pop\0R&B\0Rap\0Reggae\0Rock\0Techno\0Industrial\0Alternative\0Ska\0Death Metal\0Pranks\0Soundtrack\0Euro-Techno\0Ambient\0Trip-Hop\0Vocal\0Jazz+Funk\0Fusion\0Trance\0Classical\0Instrumental\0Acid\0House\0Game\0Sound Clip\0Gospel\0Noise\0Alternative Rock\0Bass\0Soul\0Punk\0Space\0Meditative\0Instrumental Pop\0Instrumental Rock\0Ethnic\0Gothic\0Darkwave\0Techno-Industrial\0Electronic\0Pop-Folk\0Eurodance\0Dream\0Southern Rock\0Comedy\0Cult\0Gangsta\0Top 40\0Christian Rap\0Pop/Funk\0Jungle\0Native American\0Cabaret\0New Wave\0Psychedelic\0Rave\0Showtunes\0Trailer\0Lo-Fi\0Tribal\0Acid Punk\0Acid Jazz\0Polka\0Retro\0Musical\0Rock & Roll\0Hard Rock\0";

static uint32_t syncsafe(const unsigned char *p)
{ return ((uint32_t)p[0] << 21) | ((uint32_t)p[1] << 14) | ((uint32_t)p[2] << 7) | p[3]; }
static uint32_t be32(const unsigned char *p)
{ return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3]; }
static void set_error(char *out, size_t n, const char *message)
{ if (n) snprintf(out, n, "%s", message); }
static char *copy_text(const unsigned char *p, size_t n, int encoding)
{
    size_t start = 0, end = n, i, outn = 0;
    char *s;
    /* Skip the UTF-16 byte-order mark; this small reader doesn't convert Unicode. */
    if (encoding == 1 || encoding == 2) { start = n >= 2 ? 2 : n; }
    while (end > start && (p[end-1] == 0 || p[end-1] == ' ')) --end;
    s = (char *)malloc((end-start)+1); if (!s) return NULL;
    /* Keep the source encoding bytes except UTF-16's zero bytes; this keeps the sample code small. */
    for (i=start; i<end; ++i) if (p[i] != 0) s[outn++] = (char)p[i];
    s[outn] = '\0'; return s;
}
static int frame_field(const unsigned char *id, int v22)
{
    int i; const char **ids = v22 ? id22 : TAG_FRAME_IDS;
    for (i=0;i<TAG_COUNT;++i) if (!memcmp(id, ids[i], v22 ? 3 : 4)) return i;
    return -1;
}
static int parse_v2(FILE *f, Mp3Tags *t, char *error, size_t error_size)
{
    unsigned char h[10]; uint32_t size, pos=0; unsigned char *body; int v22, ver;
    if (fseek(f,0,SEEK_SET)!=0 || fread(h,1,10,f)!=10 || memcmp(h,"ID3",3)) return 0;
    ver=h[3]; if(ver==4){set_error(error,error_size,"ID3v2.4 tags are not supported.");return -1;}
    if(ver!=2 && ver!=3){set_error(error,error_size,"Unsupported ID3v2 version.");return -1;}
    size=syncsafe(h+6); body=(unsigned char*)malloc(size?size:1); if(!body)return -1;
    if(fread(body,1,size,f)!=size){free(body);set_error(error,error_size,"Truncated ID3v2 tag.");return -1;}
    t->has_v2=1; v22=(ver==2); snprintf(t->version,sizeof(t->version),"ID3v2.%d.%d",ver,h[4]);
    while(pos+(v22?6:10)<=size) {
        unsigned char *id=body+pos; uint32_t frame_size; size_t head=v22?6:10; int field, enc=0;
        if(id[0]==0)break;
        frame_size=v22?(((uint32_t)id[3]<<16)|((uint32_t)id[4]<<8)|id[5]):be32(id+4);
        if(frame_size==0 || frame_size>size-pos-head) break;
        field=frame_field(id,v22);
        if(field>=0) {
            size_t off=head, end=head+frame_size;
            if(field==TAG_COMMENT && frame_size>=5) {
                size_t sep; enc=body[pos+head]; sep=head+4;
                if(enc==1 || enc==2) { while(sep+1<end && (body[pos+sep] || body[pos+sep+1])) sep+=2; sep+=2; }
                else { while(sep<end && body[pos+sep]) ++sep; ++sep; }
                if(sep<end) t->value[field]=copy_text(body+pos+sep,end-sep,enc);
            } else if(off<end) {
                enc=body[pos+off]; t->value[field]=copy_text(body+pos+off+1,end-off-1,enc);
            }
        }
        pos+=(uint32_t)(head+frame_size);
    }
    free(body); return 1;
}
/* ID3v1 data is stored in a fixed 128-byte block at the end of the file. */
static int read_v1(FILE *f, Mp3Tags *t)
{
    unsigned char b[128]; long end; int i;
    if (fseek(f, 0, SEEK_END) != 0) {
    return 0;
}

end = ftell(f);

if (end < 128 ||
    fseek(f, end - 128, SEEK_SET) != 0 ||
    fread(b, 1, 128, f) != 128 ||
    memcmp(b, "TAG", 3) != 0) {
    return 0;
}
    t->has_v1=1; if(!t->has_v2)strcpy(t->version,"ID3v1.0");
    { const int offsets[]={3,33,63,93}; const int lengths[]={30,30,30,4};
      for(i=0;i<4;++i) if(!t->value[i])t->value[i]=copy_text(b+offsets[i],lengths[i],0); }
    if(!t->value[TAG_COMMENT]) t->value[TAG_COMMENT]=copy_text(b+97,30,0);
    if(!t->value[TAG_GENRE] && b[127]<80) {
        const unsigned char *p=id1_genre; unsigned int k=0;
        while(k<b[127] && p<id1_genre+sizeof(id1_genre)) { p+=(strlen((const char*)p)+1); ++k; }
        if(k==b[127] && p<id1_genre+sizeof(id1_genre)) t->value[TAG_GENRE]=copy_text(p,strlen((const char*)p),0);
    }
    return 1;
}
int read_mp3_tags(const char *path, Mp3Tags *tags, char *error, size_t error_size)
{
    FILE *f=fopen(path,"rb"); int r, v1;
    memset(tags,0,sizeof(*tags)); if(!f){set_error(error,error_size,"Could not open MP3 file.");return 0;}
    r=parse_v2(f,tags,error,error_size); if(r<0){fclose(f);return 0;}
    v1=read_v1(f,tags); fclose(f);
    if(!r && !v1){set_error(error,error_size,"No ID3v1 or supported ID3v2 tag was found.");return 0;}
    return 1;
}
void free_mp3_tags(Mp3Tags *tags)
{ int i; for(i=0;i<TAG_COUNT;++i)free(tags->value[i]); memset(tags,0,sizeof(*tags)); }

/* Rebuilds the ID3v2.3 frames while preserving other frames and the audio bytes. */
