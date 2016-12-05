
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>
#include "SDL_3ds.h"
#include "stb_vorbis.h"


#define MUSIC_GAIN 0 //0.3

bool soundEnabled;

typedef struct chStatus {
	u64 playlen;	
	u64 playstart;		
} chStatus;

chStatus soundchannels[8];

FSOUND_SAMPLE SFX[NUMSFX];

int SFXMasterVolume = 255;
int MasterVolume = 255;
int frequency=0;
int channel=0;

const char* stdMixErr = "SDL MIX ERROR";

int getFreeChannel()
{
	int startchannel = channel; 
	do
	{
		channel = (channel+1)%7;
		if (svcGetSystemTick()> soundchannels[channel].playlen + soundchannels[channel].playstart) return channel;
	} while(channel != startchannel);
	return -1;
}

void soundInit()
{
	int i;
	for(i=0;i<NUMSFX;i++)
	{
		SFX[i].used=false;
	}

	for(i=0;i<8;i++)
	{
		soundchannels[i].playlen=0;
		soundchannels[i].playstart=0;
	}


	if(csndInit()==0)soundEnabled=true;
	else soundEnabled=false;
}

void soundClose()
{
	int i;
	for(i=0;i<NUMSFX;i++)
	{
		if(SFX[i].used)
		{
			if(SFX[i].data)
			{
				linearFree(SFX[i].data);
				SFX[i].data=NULL;
			}
			SFX[i].used=false;
		}
	}
	if(soundEnabled)csndExit();
}

FILE* openFile(const char* fn, const char* mode)
{
	if(!fn || !mode)return NULL;
	return fopen(fn, mode);
}

void* bufferizeFile(const char* filename, u32* size, bool binary, bool linear)
{
	FILE* file;
	
	if(!binary)file = openFile(filename, "r");
	else file = openFile(filename, "rb");
	
	if(!file) return NULL;
	
	u8* buffer;
	long lsize;
	fseek (file, 0 , SEEK_END);
	lsize = ftell (file);
	rewind (file);
	if(linear)buffer=(u8*)linearMemAlign(lsize, 0x80);
	else buffer=(u8*)malloc(lsize);
	if(size)*size=lsize;
	
	if(!buffer)
	{
		fclose(file);
		return NULL;
	}
		
	fread(buffer, 1, lsize, file);
	fclose(file);
	return buffer;
}

int FSOUND_Init(u32 freq, u32 bps, u32 unkn)
{

	frequency = freq;
	
	return 0;//soundEnabled;
}

void initSFX(FMUSIC_MODULE* s)
{
	if(!s)return;

	s->data=NULL;
	s->size=0;
	s->used=true;
	s->loop=false;
}

void load_SFX(FMUSIC_MODULE* s, const char* filename, u32 format)
{
	if(!s)return;

	initSFX(s);

	s->data=(u8*) bufferizeFile(filename, &s->size, true, true);
	s->format=format;
	s->freq=frequency;
}

int FSOUND_GetSFXMasterVolume()
{
	return SFXMasterVolume;
}

int FMUSIC_GetMasterVolume(FMUSIC_MODULE* s)
{
	return MasterVolume;
}

void FMUSIC_SetMasterVolume(FMUSIC_MODULE* s, u8 volume)
{
	MasterVolume = volume;
	CSND_SetVol(15, CSND_VOL((float)MasterVolume/MIX_MAX_VOLUME, 0), CSND_VOL((float)MasterVolume/MIX_MAX_VOLUME, 0));
	csndExecCmds(0);
}

void FSOUND_SetSFXMasterVolume(u8 volson)
{
	SFXMasterVolume = volson;
}

void FSOUND_PlaySound(int ch,FSOUND_SAMPLE* s)
{
	if(!s || !s->used || !s->data || !soundEnabled || SFXMasterVolume == 0)return;

	int freech = getFreeChannel();
	if (freech>=0)
	{
		soundchannels[freech].playlen= ((u64)s->size * 1000 * TICKS_PER_MSEC) / s->freq;
		soundchannels[freech].playstart=svcGetSystemTick();
//		csndPlaySound(freech+8, s->format, s->freq, 1.0, 0.0, (u32*)s->data, (u32*)s->data, s->size);
		csndPlaySound(freech+8, s->format, s->freq, (float)SFXMasterVolume/MIX_MAX_VOLUME, 0.0, (u32*)s->data, (u32*)s->data, s->size);
	}
}

void FMUSIC_StopSong(FMUSIC_MODULE* s)
{
	CSND_SetPlayState(15, 0);//Stop music audio playback.
	csndExecCmds(0);
}

void FMUSIC_PlaySong(FMUSIC_MODULE* s)
{
	int flag;
	if(!s || !s->used || !s->data || !soundEnabled) return;
	flag = s->format;
	if(s->loop) flag |= SOUND_REPEAT;
	soundchannels[7].playlen= ((u64)s->size * 1000 * TICKS_PER_MSEC) / s->freq;
	soundchannels[7].playstart=svcGetSystemTick();
//	csndPlaySound(15, flag,s->freq, 1.0, 0.0, (u32*)s->data, (u32*)s->data, s->size);
	csndPlaySound(15, flag,s->freq, (float)MasterVolume/MIX_MAX_VOLUME, 0.0, (u32*)s->data, (u32*)s->data, s->size);
}

int dumpcount = 0;

FSOUND_SAMPLE* FSOUND_Sample_Load(int flag, const char * f,int a, int b, int c)
{
	int i;
	for(i=0;i<NUMSFX;i++)
	{
		if(!SFX[i].used)
		{
printf("Load %s...",f);
			int channels=1;
			s16 * buffer;
			int freq=11025;
			u32 scount;
			int len = stb_vorbis_decode_filename(f, &channels, &freq, &buffer);
			if (len<=0) {
printf("ko\n");
				return NULL;
			}
			if(channels==2) {
				SFX[i].data = (u8*) linearMemAlign(len *sizeof(s16), 0x80);
				if(!SFX[i].data) {
					if(buffer) free(buffer);
printf("ko\n");
					return NULL;
				}
				for (scount=0;scount<len;scount++) 
					((s16*)SFX[i].data)[scount]=buffer[scount*2];
			} else if (channels==1) {
				SFX[i].data = (u8*) linearMemAlign(len*sizeof(s16), 0x80);
				if(!SFX[i].data) {
					if(buffer) free(buffer);
printf("ko\n");
					return NULL;
				}
//				for (scount=0;scount<len;scount++) 
//					((s16*)SFX[i].data)[scount]=buffer[scount];
				memcpy(SFX[i].data,(u8*)buffer, len*sizeof(s16)); 
			} else {
				if(buffer) free(buffer);
printf("ko\n");
				return NULL;
			}
#if 0 // write on disk the decoded stream as raw pcm signed 16bit samples for debugging 
	char dumpbuf[255];
	sprintf(dumpbuf, "dump%i.raw", dumpcount++);
	if( buffer )
	{
		FILE* pf = fopen( dumpbuf, "wb" );
		if( pf )
		{
			fwrite( (u8*)SFX[i].data, 1, len*sizeof(s16), pf );
			fclose( pf );
		}
	}
#endif
			free(buffer);
			SFX[i].size= len;
			SFX[i].freq = freq;
			SFX[i].format = SOUND_FORMAT_16BIT;
			SFX[i].used = true;
			SFX[i].loop=false;
printf("ok\n");
			return &SFX[i];
		}
	}
	return NULL;
}

FMUSIC_MODULE* FMUSIC_LoadSong(const char * f)
{
	return FSOUND_Sample_Load(0, f,0, 0, 0);
}

void FSOUND_Close(){
	soundClose();
}


void FMUSIC_SetLooping(FMUSIC_MODULE* s, bool flag)
{
	if (s)
		s->loop=flag;
}

void FSOUND_Sample_Free(FSOUND_SAMPLE* s)
{
	if(s) {
		if (s->data)
			linearFree(s->data);
		s->size=0;
		s->used=false;
		s->loop=false;
	}
}


void FMUSIC_FreeSong(FMUSIC_MODULE* s)
{
	if(s) {
		if (s->data)
			linearFree(s->data);
		s->size=0;
		s->used=false;
		s->loop=false;
	}
}

int Mix_ReserveChannels(int c)
{
	return c;
}

int Mix_AllocateChannels(int c)
{
	return c;
}

void Mix_SetPanning(int c, int a, int b){};
void Mix_UnregisterAllEffects(int c){};

int Mix_PlayChannel(int ch, Mix_Chunk *s, int loops)
{
	if(!s || !s->used || !s->data || !soundEnabled || SFXMasterVolume == 0) return -1;

	if(loops) s->format |= SOUND_REPEAT;
	else s->format &= ~SOUND_REPEAT;
	int freech = getFreeChannel();
	if (freech>=0)
	{
		soundchannels[freech].playlen= ((u64)s->size * 1000 * TICKS_PER_MSEC) / s->freq;
		soundchannels[freech].playstart=svcGetSystemTick();
//		csndPlaySound(freech+8, s->format, s->freq, 1.0, 0.0, (u32*)s->data, (u32*)s->data, s->size);
		csndPlaySound(freech+8, s->format, s->freq, (float)SFXMasterVolume/MIX_MAX_VOLUME, 0.0, (u32*)s->data, (u32*)s->data, s->size);
		return freech;
	} else
		return 0;
}

int Mix_PlayMusic(Mix_Music * s , int loops )
{
	int flag;
	if(!s || !s->used || !s->data || !soundEnabled) return -1;
	flag = s->format;
	if (loops) flag |= SOUND_REPEAT;
	soundchannels[7].playlen= ((u64)s->size * 1000 * TICKS_PER_MSEC) / s->freq;
	soundchannels[7].playstart=svcGetSystemTick();
//	csndPlaySound(15, flag, s->freq, 1.0, 0.0, (u32*)s->data, (u32*)s->data, s->size);
	csndPlaySound(15, flag, s->freq, (float)MasterVolume/MIX_MAX_VOLUME, 0.0, (u32*)s->data, (u32*)s->data, s->size);
	return 15;
}

int Mix_PlayingMusic()
{
	return (svcGetSystemTick()< soundchannels[7].playlen + soundchannels[7].playstart)?1:0;
}

int Mix_OpenAudio( int audio_rate, u16 audio_format, int audio_channels, int bufsize ){
	return FSOUND_Init(audio_rate, 0, 0);
}

void Mix_VolumeMusic( int vol ){
	MasterVolume = (vol>MIX_MAX_VOLUME) ? MIX_MAX_VOLUME:vol;
	CSND_SetVol(15, CSND_VOL((float)MasterVolume/MIX_MAX_VOLUME, 0), CSND_VOL((float)MasterVolume/MIX_MAX_VOLUME, 0));
	csndExecCmds(0);
}
void Mix_Volume(  int c, int vol ){
	SFXMasterVolume = (vol>MIX_MAX_VOLUME) ? MIX_MAX_VOLUME:vol;
}

Mix_Chunk * Mix_LoadWAV(const char * f) {
	return FSOUND_Sample_Load(0, f,0, 0, 0);
}

Mix_Music * Mix_LoadMUS(const char *f){
	u32 filesize;
	u8* buffer = (u8*) bufferizeFile(f, &filesize, true, true);
	SDL_RWops* musRW = SDL_RWFromMem(buffer, filesize);
	Mix_Music * mus = Mix_LoadMUS_RW(musRW);
	SDL_FreeRW(musRW);
	return mus;
}

const char * Mix_GetError(){
	return stdMixErr;
}

void Mix_CloseAudio(){
	soundClose();
}

void Mix_FadeInMusic(Mix_Music* s, int a, int b){
	Mix_PlayMusic(s , 1);
}

void Mix_FadeOutMusic(int a){
	Mix_HaltMusic();
}

Mix_Chunk * Mix_LoadWAV_RW(SDL_RWops* buffer, int loops)
{
	return NULL;
}

Mix_Music * Mix_LoadMUS_RW(SDL_RWops* buffer)
{
	return NULL;
}

void Mix_FreeMusic(Mix_Music* s)
{
	if(s) {
		if (s->data)
			linearFree(s->data);
		s->size=0;
		s->used=false;
		s->loop=false;
	}
}

void Mix_FreeChunk(Mix_Chunk* s)
{
	if(s) {
		if (s->data)
			linearFree(s->data);
		s->size=0;
		s->used=false;
		s->loop=false;
	}
}

void Mix_HookMusicFinished(void* p){}

void Mix_HaltMusic(void)
{
	CSND_SetPlayState(15, 0);//Stop music audio playback.
	csndExecCmds(0);
	soundchannels[7].playlen= 0;
	soundchannels[7].playstart=0;
}

void Mix_PauseMusic()
{
	CSND_SetPlayState(15, 0);//Stop music audio playback.
	csndExecCmds(0);
}

void Mix_ResumeMusic()
{
	CSND_SetPlayState(15, 1);//Resume music audio playback.
	csndExecCmds(0);
}

void Mix_HaltChannel(int ch)
{
	if(ch >7 && ch <16) {
		CSND_SetPlayState(ch, 0);//Stop music audio playback.
		csndExecCmds(0);
		soundchannels[ch-8].playlen= 0;
		soundchannels[ch-8].playstart=0;
	}
}

void Mix_Pause(int ch)
{
	if(ch >7 && ch <16) {
		CSND_SetPlayState(ch, 0);//Stop music audio playback.
		csndExecCmds(0);
	}
}

void Mix_Resume(int ch)
{
	if(ch >7 && ch <16) {
		CSND_SetPlayState(ch, 1);//Resume music audio playback.
		csndExecCmds(0);
	}
}

int Mix_Playing(int ch)
{
	if(ch >7 && ch <16) {
		return (svcGetSystemTick()< soundchannels[ch-8].playlen + soundchannels[ch-8].playstart)?1:0;
	}
	return 0;
}

int Mix_Init(int flag){
	return flag;
}

void Mix_Quit(void){
}

