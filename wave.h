#pragma once

#include <vector>
#include <string>


// define
const unsigned char STR_RIFF[] = {'R', 'I', 'F', 'F'};
const unsigned char STR_WAVE[] = {'W', 'A', 'V', 'E'};
const unsigned char STR_fmt[]  = {'f', 'm', 't', ' '};
const unsigned char STR_data[] = {'d', 'a', 't', 'a'};

const uint16_t WAV_MONORAL = 1;
const uint16_t WAV_STEREO  = 2;
const uint16_t WAV_DEFAULT_SPS = 44100;
const uint16_t WAV_DEFAULT_BPS = 16;

const unsigned int WAV_UNSIGNED_8BIT_MAX  = 0x1<<8;
const unsigned int WAV_UNSIGNED_16BIT_MAX = 0x1<<16;
const unsigned int WAV_UNSIGNED_24BIT_MAX = 0x1<<24;
// const unsigned int WAV_UNSIGNED_32BIT_MAX = 0x1<<32;// 整数

const int WAV_SIGNED_8BIT_MAX  = 0x1<<7;
const int WAV_SIGNED_16BIT_MAX = 0x1<<15;
const int WAV_SIGNED_24BIT_MAX = 0x1<<23;
// const int WAV_SIGNED_32BIT_MAX = 0x1<<31;// 整数

const std::string AppOutName = "_out";

#pragma pack(1)
struct tagHeader {
	char            hdrRiff[4];         // 'RIFF'
	unsigned int    sizeOfFile;         // ファイルサイズ - 8
	char            hdrWave[4];         // 'WAVE'
};
#pragma pack()

struct tagChunk {
	unsigned char   hdrFmtData[4];      // 'fmt ' or 'data'
	unsigned int    sizeOfFmtData;      // sizeof(PCMWAVEFORMAT) or Waveデーターサイズ
};

#pragma pack(1)
struct tagWaveFormatPcm {
	uint16_t  formatTag;                // WAVE_FORMAT_PCM
    uint16_t  channels;                 // number of channels
	unsigned int    samplesPerSec;      // sampling rate
    unsigned int    bytesPerSec;        // samplesPerSec * channels * (bitsPerSample/8)
	uint16_t  blockAlign;               // block align
	uint16_t  bitsPerSample;            // bits per sampling
};
#pragma pack()

#pragma pack(1)
struct tagWaveFormat {
	uint16_t  formatTag;            // WAVE_FORMAT_PCM
	uint16_t  channels;             // number of channels
	unsigned int    samplesPerSec;  // sampling rate
	unsigned int    bytesPerSec;    // samplesPerSec * channels * (bitsPerSample/8)
	uint16_t  blockAlign;           // block align
	uint16_t  bitsPerSample;        // bits per sampling
};	// PCMWAVEFORMAT
#pragma pack()

#pragma pack(1)
struct tagWaveFileHeader {
	unsigned char   hdrRiff[4];         // 'RIFF'
	unsigned int    sizeOfFile;         // ファイルサイズ - 8
	unsigned char   hdrWave[4];         // 'WAVE'
	unsigned char   hdrFmt[4];          // 'fmt '
	unsigned int    sizeOfFmt;          // sizeof( PCMWAVEFORMAT )
	tagWaveFormat	wavfmt;				// Wave Format
	unsigned char   hdrData[4];         // 'data'
	unsigned int    sizeOfData;         // Waveデーターサイズ	
};
#pragma pack()



class Wave {
 public:
	// prototype
	Wave();
	~Wave();

	// INPUT/OUTPUT FILE
	// Read Wave File
	int InputWave(const std::string filename);
	// Read Text File *
	int InputText(const std::string filename);
	// Output Wave
	void OutputWave(const std::string app = AppOutName/*appendname(default _out)*/);
	// Output txt *
	// void OutputText();

	// SET
	// Set Name
	void SetName(const std::string filename);
	// Set MonoData
	void SetData(const std::vector<double> mono );
	// Set StereoData
	void SetData(const std::vector<double> stereoL ,const std::vector<double> stereoR );
	// Create Mono Wave
	void CreateWave(const std::vector<double> mono, const uint16_t samples_per_sec = WAV_DEFAULT_SPS,
		const uint16_t bits_per_sample = WAV_DEFAULT_BPS);
	// Create Stereo Wave
	void CreateWave(std::vector<double> stereoL, std::vector<double> stereoR, const uint16_t samples_per_sec = WAV_DEFAULT_SPS, const uint16_t bits_per_sample = WAV_DEFAULT_BPS);

	// EDIT
	// Stereo to Mono
	void StereoToMono();
	// Normalize
	void Normalize();
	// Sinc Resampling
	void ResamplingSinc(const uint16_t resampling, const int sincLengthHarf);

	// GET
	// Get MonoData
	void GetData(std::vector<double> *mono);
	// Get StereoData
	void GetData(std::vector<double> *stereoL, std::vector<double> *stereoR);
	// std::vector<int> GetFreqFluc();


 private:
	tagHeader			header;			// Header(WAVE/RIFF/DATA)
	tagChunk			chunk;			// Chunk(fmt/data)
	tagWaveFormatPcm	wavfmtpcm;		// WaveFormatPCM()
	tagWaveFileHeader	wavfhdr;		// WaveFileHeader

	std::string		foldername;
	std::string		wavename;			// wavename
	uint16_t  bitsPerSample;            // 8/16/24/32 bits
    unsigned int    sizeOfData;         // Waveデーターサイズ
    uint16_t  channels;                 // チャンネル数
	uint16_t  samplesPerSec;            // 44.1/48 kHz
    unsigned int    bytesPerSec;        // バイト数/sec
    int64_t            posOfData;        // position of begnning of WAV datas
	std::vector<double>  monodata;      // mono wavedata
	std::vector<double>  ldata;         // left wavedata
	std::vector<double>  rdata;         // right wavedata
	bool flag;

	void SetHeader();
	// InputWave
	int WavHdrRead();
	const int ReadfmtChunk(std::ifstream* fin);
	int DumpData();
	// Stereo Signed
	int Dump8BitStereoWave();
	int Dump16BitStereoWave();
	int Dump24BitStereoWave();
	// int Dump32BitStereoWave();
	// int Dump32BitFloatStereoWave();
	// Mono Signed
	int Dump8BitMonoWave();
	int Dump16BitMonoWave();
	int Dump24BitMonoWave();
	// int Dump32BitMonoWave();
	// int Dump32BitFloatMonoWave();
	// Resampling
	double Sinc(const double x);
};
