#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <fstream>

#include "Wave.h"


Wave::Wave() {
	wavename = "temp.wav";
	flag = false;
}

Wave::~Wave() {
}

double Wave::Sinc(const double x) {
	if (x==0.0)
		return 1.0;
	else
		return sin(x)/x;
}

int Wave::InputWave(const std::string filename) {
	if (filename.compare(filename.size()-4,4, ".wav") == 0) {
		ldata.clear();
		rdata.clear();
		monodata.clear();

		wavename = filename;
		WavHdrRead();
		DumpData();
		SetHeader();
		flag = true;
	}
    else {
		std::cerr << "error .wav" << std::endl;
		return -1;
	}
	return 0;
}

int Wave::WavHdrRead() {
	std::ifstream fin;
    long cursor, len;
	
	fin.open(wavename, std::ios::binary);
	if( !fin ) {
	    std::cerr << "ファイルを開けませんでした." << std::endl;
		return -1;
	}
    // ヘッダ情報
	fin.read( (char*)&header,sizeof(header));
	if( fin.bad() ) {
		std::cerr << "読み込みエラー" << std::endl;
		exit( 0 );
    }

    // WAVE ヘッダ情報
    if (memcmp(header.hdrWave, STR_WAVE, 4) != 0) {
        std::cerr << "'WAVE' が無い." << std::endl;
        return -1;
    }

    if(memcmp(&header.hdrRiff, STR_RIFF, 4) != 0) {
        std::cerr << "'RIFF' フォーマットでない." << std::endl;
        return -1;
    }

    // 4Byte これ以降のバイト数 = (ファイルサイズ - 8)(Byte)
    len = header.sizeOfFile;

    // チャンク情報
	while( !fin.eof() ) {
		fin.read((char*)&chunk, sizeof(chunk));
		if( fin.bad() ) {
			std::cerr << "読み込みエラー" << std::endl;
			exit( 0 );
		}

		if(memcmp( chunk.hdrFmtData, STR_fmt, sizeof chunk.hdrFmtData) == 0) {
			len = chunk.sizeOfFmtData;
			cursor = fin.tellg();
			if(ReadfmtChunk(&fin)!=0)   
				return -1;
			samplesPerSec	= wavfmtpcm.samplesPerSec;
			bitsPerSample	= wavfmtpcm.bitsPerSample;
			channels		= wavfmtpcm.channels;
			bytesPerSec		= wavfmtpcm.bytesPerSec;
			fin.seekg(cursor + len,std::ios_base::beg);
		}
		else if(memcmp(chunk.hdrFmtData, STR_data, 4) == 0) {
            sizeOfData = chunk.sizeOfFmtData;
			posOfData = fin.tellg();
			fin.seekg(sizeOfData + posOfData - 4,std::ios_base::beg);
        }
		else {
            len = chunk.sizeOfFmtData;
			cursor = fin.tellg();
			fin.seekg(cursor + len,std::ios::beg);
        }
	}

    return 0;
}

const int Wave::ReadfmtChunk(std::ifstream* fin) {
	fin->read( (char*) &wavfmtpcm, sizeof(tagWaveFormatPcm));
	if( fin->bad() ) {
		std::cerr << "読み込みエラー" << std::endl;
		exit( 0 );
	}

    std::cout << "    formatTag: " << wavfmtpcm.formatTag << " (1 = PCM)" << std::endl;
    std::cout << "    channels: " << wavfmtpcm.channels << "[channel]" << std::endl;
    std::cout << "    samplesPerSec: " << wavfmtpcm.samplesPerSec << "[Hz]"	<< std::endl;
    std::cout << "    bitsPerSample: " << wavfmtpcm.bitsPerSample << "[bits/sample]" << std::endl;

    if(wavfmtpcm.formatTag != 1) {
		std::cerr << "\nこのプログラムは無圧縮PCMのみを対象とします." << std::endl;
		std::cerr << "このwavファイルの形式は " << wavfmtpcm.formatTag << " です." << std::endl;
        return -1;
    }
    if(wavfmtpcm.bitsPerSample != 8 && wavfmtpcm.bitsPerSample != 16 && wavfmtpcm.bitsPerSample != 24 && wavfmtpcm.bitsPerSample != 32) {
		std::cerr << "\nこのプログラムは8/16/24ビットサンプリングされたものを対象とします." << std::endl;
		std::cerr << "このwavファイルの bits/secは " << wavfmtpcm.bitsPerSample << " です." << std::endl;
        return -1;
    }

    return 0;
}
/*--------------------------------------------------------------------------
 * wav データをダンプ
 */

int Wave::DumpData() {
	uint16_t bytesPerSingleCh;

	bytesPerSingleCh = bitsPerSample / 8;

	if(channels == 1) {
		if(bytesPerSingleCh == 1)
			return Dump8BitMonoWave();
		else if(bytesPerSingleCh == 2)
			return Dump16BitMonoWave();
		else if(bytesPerSingleCh == 3)
			return Dump24BitMonoWave();
		else
			return -1;
	}
	else if(channels == 2) {
		if(bytesPerSingleCh == 1)
			return Dump8BitStereoWave();
		else if(bytesPerSingleCh == 2)
			return Dump16BitStereoWave();
		else if(bytesPerSingleCh == 3)
			return Dump24BitStereoWave();
		else
			return -1;
	}
	else {
		return -1;
    }
    return 0;
}

/*--------------------------------------------------------------------------
 * 8 bits/sampling Stereo
 */
int Wave::Dump8BitStereoWave() {
    unsigned int  i;
    char l,r;
	std::ifstream fin;

	fin.open(wavename, std::ios::binary);

	fin.seekg( posOfData,std::ios::beg); //元ファイルのデータ開始部分へ

	for (i = 0; i < (sizeOfData/2) / sizeof (char); i++) {
		fin.read( (char*) &l, sizeof(char));
		fin.read( (char*) &r, sizeof(char));

		ldata.push_back((double)l/(double)WAV_SIGNED_8BIT_MAX);
		rdata.push_back((double)r/(double)WAV_SIGNED_8BIT_MAX);
    }
    return 0;
}

/*--------------------------------------------------------------------------
 * 8 bits/sampling Mono
 */
int Wave::Dump8BitMonoWave() {
    unsigned int i;
	char m;
	std::ifstream fin;

	fin.open(wavename, std::ios::binary);

	fin.seekg( posOfData,std::ios::beg); //元ファイルのデータ開始部分へ

	for (i = 0; i < sizeOfData / sizeof (char); i++) {
        fin.read( (char*) &m, sizeof(char));

		monodata.push_back((double)m/(double)WAV_SIGNED_16BIT_MAX);
    }
    return 0;
}
/*--------------------------------------------------------------------------
 * 16 bits/sampling Stereo
 */
int Wave::Dump16BitStereoWave() {
    unsigned int  i;
	short l,r;
	std::ifstream fin;

	fin.open(wavename, std::ios::binary);

	fin.seekg( posOfData,std::ios::beg); //元ファイルのデータ開始部分へ

	for (i = 0; i < (sizeOfData/2)/ ( sizeof (char) * 2 ); i++) {
		fin.read( (char*)&l, sizeof(char) * 2 );
		fin.read( (char*)&r, sizeof(char) * 2 );

		ldata.push_back((double)l/(double)WAV_SIGNED_16BIT_MAX);
		rdata.push_back((double)r/(double)WAV_SIGNED_16BIT_MAX);
    }
    return 0;
}

/*--------------------------------------------------------------------------
 * 16 bits/sampling Mono
 */
int Wave::Dump16BitMonoWave() {
    unsigned int  i;
	short m;
	std::ifstream fin;

	fin.open(wavename, std::ios::binary);

	fin.seekg( posOfData,std::ios::beg); //元ファイルのデータ開始部分へ

	for (i = 0; i < sizeOfData / ( sizeof (char) * 2 ); i++) {
        fin.read( (char*) &m, sizeof(char) * 2 );

		monodata.push_back((double)m/(double)WAV_SIGNED_16BIT_MAX);
    }
    return 0;
}

/*--------------------------------------------------------------------------
 * 24 bits/sampling Stereo
 */
int Wave::Dump24BitStereoWave() {
    unsigned int  i;
    int l,r;
	std::ifstream fin;

	fin.open(wavename, std::ios::binary);

	fin.seekg( posOfData,std::ios::beg); //元ファイルのデータ開始部分へ

	for (i = 0; i < (sizeOfData/2) / ( sizeof (char) * 3 ); i++) {
		fin.read( (char*) &l, sizeof(char) * 3 );
		fin.read( (char*) &r, sizeof(char) * 3 );

		ldata.push_back((double)l/(double)WAV_SIGNED_24BIT_MAX);
		rdata.push_back((double)r/(double)WAV_SIGNED_24BIT_MAX);
    }
    return 0;
}

/*--------------------------------------------------------------------------
 * 24 bits/sampling Mono
 */
int Wave::Dump24BitMonoWave() {
    unsigned int  i;
	int m;
	std::ifstream fin;

	fin.open(wavename, std::ios::binary);

	fin.seekg( posOfData,std::ios::beg); //元ファイルのデータ開始部分へ

	for (i = 0; i < sizeOfData / ( sizeof (char) * 3 ); i++) {
        fin.read( (char*) &m, sizeof(char) * 3 );

		monodata.push_back((double)m/(double)WAV_SIGNED_24BIT_MAX);
    }
    return 0;
}

/*--------------------------------------------------------------------------
 * ヘッダの作成
 */
void Wave::SetHeader() {
    uint16_t bytes;

	memcpy(wavfhdr.hdrRiff, STR_RIFF, sizeof(wavfhdr.hdrRiff));		    // RIFF
    wavfhdr.sizeOfFile = sizeOfData+sizeof(tagWaveFileHeader) - 8;    	// ファイルサイズ
	memcpy(wavfhdr.hdrWave, STR_WAVE, sizeof(wavfhdr.hdrWave)); 		// WAVE
	memcpy(wavfhdr.hdrFmt, STR_fmt, sizeof(wavfhdr.hdrFmt));    		// fmt
	wavfhdr.sizeOfFmt = sizeof wavfhdr.wavfmt;			         		// sizeof( PCMWAVEFORMAT )
	wavfhdr.wavfmt.formatTag = 1;				            			// WAVE_FORMAT_PCM
	wavfhdr.wavfmt.channels = channels;				                	// mono/stereo
	wavfhdr.wavfmt.samplesPerSec = samplesPerSec;			        	// Hz
    bytes = bitsPerSample / 8;								        	// bytes/sec
	wavfhdr.wavfmt.bytesPerSec = bytes * channels * samplesPerSec;		// byte/サンプル*チャンネル
	wavfhdr.wavfmt.blockAlign = bytes * channels;			            // block_Size
	wavfhdr.wavfmt.bitsPerSample = bitsPerSample;				        // 16 bit / sample
	memcpy(wavfhdr.hdrData, STR_data, sizeof(wavfhdr.hdrData));	    	// data
    wavfhdr.sizeOfData = sizeOfData;		                			// データ長 (byte)
}

/*--------------------------------------------------------------------------
 * CreateWaveClass Mono
 */
void Wave::CreateWave(const std::vector<double> mono,const uint16_t samples_per_sec, const uint16_t bits_per_sample) {
	monodata.clear();
	ldata.clear();
	rdata.clear();

	uint16_t bytes;
	channels = 1;
	monodata = mono;
	samplesPerSec = samples_per_sec;
	bitsPerSample = bits_per_sample;

	memcpy(wavfhdr.hdrRiff, STR_RIFF, sizeof(wavfhdr.hdrRiff));	    	// RIFF
    wavfhdr.sizeOfFile = sizeOfData+sizeof(tagWaveFileHeader) - 8;  	// ファイルサイズ
	memcpy(wavfhdr.hdrWave, STR_WAVE, sizeof(wavfhdr.hdrWave));	    	// WAVE
	memcpy(wavfhdr.hdrFmt, STR_fmt, sizeof(wavfhdr.hdrFmt));	    	// fmt
	wavfhdr.sizeOfFmt = sizeof wavfhdr.wavfmt;				        	// sizeof( PCMWAVEFORMAT )
	wavfhdr.wavfmt.formatTag = 1;						            	// WAVE_FORMAT_PCM
	wavfhdr.wavfmt.channels = channels;				                	// mono/stereo
	wavfhdr.wavfmt.samplesPerSec = samplesPerSec;				        // Hz
    bytes = bitsPerSample / 8;								        	// bytes/sec
	wavfhdr.wavfmt.bytesPerSec = bytes * channels * samplesPerSec;		// byte/サンプル*チャンネル
	wavfhdr.wavfmt.blockAlign = bytes * channels;		            	// block_Size
	wavfhdr.wavfmt.bitsPerSample = bitsPerSample;		        		// 16 bit / sample
	memcpy(wavfhdr.hdrData, STR_data, sizeof(wavfhdr.hdrData));	    	// data

	sizeOfData = mono.size() * wavfhdr.wavfmt.blockAlign;
    wavfhdr.sizeOfData = sizeOfData;
	flag = true;
}

/*--------------------------------------------------------------------------
 * CreateWaveClass stereo
 */
void Wave::CreateWave(std::vector<double> stereoL ,std::vector<double> stereoR, const uint16_t samples_per_sec,const uint16_t bits_per_sample) {
	monodata.clear();
	ldata.clear();
	rdata.clear();

	uint16_t bytes;

	if( stereoL.size() > stereoR.size() ) {
		stereoR.resize( stereoL.size() );
	}
	else if( stereoL.size() < stereoR.size() ) {
		stereoL.resize( stereoR.size() );
	}

	channels = 2;
	ldata = stereoL;
	rdata = stereoR;
	samplesPerSec = samples_per_sec;
	bitsPerSample = bits_per_sample;

	memcpy(wavfhdr.hdrRiff, STR_RIFF, sizeof(wavfhdr.hdrRiff));	    	// RIFF
    wavfhdr.sizeOfFile = sizeOfData+sizeof(tagWaveFileHeader) - 8;  	// ファイルサイズ
	memcpy(wavfhdr.hdrWave, STR_WAVE, sizeof(wavfhdr.hdrWave));	    	// WAVE
	memcpy(wavfhdr.hdrFmt, STR_fmt, sizeof(wavfhdr.hdrFmt));    		// fmt
	wavfhdr.sizeOfFmt = sizeof wavfhdr.wavfmt;	        				// sizeof( PCMWAVEFORMAT )
	wavfhdr.wavfmt.formatTag = 1;					            		// WAVE_FORMAT_PCM
	wavfhdr.wavfmt.channels = channels;				                	// mono/stereo
	wavfhdr.wavfmt.samplesPerSec = samplesPerSec;			        	// Hz
    bytes = bitsPerSample / 8;								        	// bytes/sec
	wavfhdr.wavfmt.bytesPerSec = bytes * channels * samplesPerSec;		// byte/サンプル*チャンネル
	wavfhdr.wavfmt.blockAlign = bytes * channels;	            		// block_Size
	wavfhdr.wavfmt.bitsPerSample = bitsPerSample;	           			// 16 bit / sample
	memcpy(wavfhdr.hdrData, STR_data, sizeof(wavfhdr.hdrData));	    	// data

	sizeOfData = stereoL.size() * wavfhdr.wavfmt.blockAlign;
    wavfhdr.sizeOfData = sizeOfData;
	flag = true;
}

/*--------------------------------------------------------------------------
 * Output Wave File
 */
void Wave::OutputWave(const std::string app) {
	if(flag == false) {
		std::cerr << "Please Create or Input Wave." << std::endl;
	}
	else {
		int max;
		int mono, stereoL, stereoR;
		std::ofstream fout;

		if (wavfhdr.wavfmt.bitsPerSample == 8) {
			max = WAV_SIGNED_8BIT_MAX;
		}
		else if (wavfhdr.wavfmt.bitsPerSample == 16) {
			max = WAV_SIGNED_16BIT_MAX;
		}
		else if (wavfhdr.wavfmt.bitsPerSample == 24) {
			max = WAV_SIGNED_24BIT_MAX;
		}
		else {
			std::cerr << "error:bits per sample" << std::endl;
		}

		fout.open(app, std::ios::binary);

		// wav ヘッダ書き込み
		fout.write( (char*) &wavfhdr, sizeof(tagWaveFileHeader) );

		if (wavfhdr.wavfmt.channels == 1) {
			for (unsigned int i=0; i < wavfhdr.sizeOfData / wavfhdr.wavfmt.blockAlign ; i++) {
				mono = (monodata[i]) * max;
				if (mono >= max)
					mono = max-1;
                if (mono <= -max)
                    mono = -max+1;
				fout.write( (char*) &mono, sizeof(char) * ( wavfhdr.wavfmt.blockAlign / wavfhdr.wavfmt.channels ) );
			}
		}
		else {
			for (unsigned int i=0; i < wavfhdr.sizeOfData / wavfhdr.wavfmt.blockAlign ; i++) {
				stereoL= ldata[i] * max;
				stereoR= rdata[i] * max;
				if (stereoL >= max)
					stereoL = max-1;
                if (stereoL <= -max)
                    stereoL = -max+1;
				if (stereoR >= max)
					stereoR = max-1;
                if (stereoR <= -max)
                    stereoR = -max+1;

				fout.write( (char*) &stereoL, sizeof(char) * ( wavfhdr.wavfmt.blockAlign / wavfhdr.wavfmt.channels ) );
				fout.write( (char*) &stereoR, sizeof(char) * ( wavfhdr.wavfmt.blockAlign / wavfhdr.wavfmt.channels ) );
			}
		}
	}
}

/*--------------------------------------------------------------------------
 * Stereo to Mono
 */
void Wave::StereoToMono() {
	if(flag == false) {
		std::cerr << "Please Create or Input Wave." << std::endl;
	}
	else {
		if (channels == 1) {
			std::cout << "モノラルデータです." << std::endl;
		}
		else {
			monodata.resize( ldata.size() );
			std::cout<< "Stereo to Monoral." << std::endl;
			for(int i = 0; i < ldata.size(); i++) {
				monodata[i] = (ldata[i]+rdata[i])/2;
			}
			channels = 1;
			sizeOfData /=2;
			ldata.clear();
			rdata.clear();
			SetHeader();
		}
	}
}

/*--------------------------------------------------------------------------
 * Normalize
 */
void Wave::Normalize() {
	if (flag == false) {
		std::cerr << "Please Create or Input Wave." << std::endl;
	}
	else {
		double max = 0.0;
		double lmax = 0.0,rmax = 0.0;
		if ( channels == 1 ) {
			for (int i = 0; i < monodata.size(); i++) {
				if (max < fabs(monodata[i]) )
					max = fabs(monodata[i]);
			}
			for (int i = 0; i < monodata.size(); i++) {
				monodata[i] /= max;
			}
		}
		else if ( channels == 2 ) {
			for (int i = 0; i < ldata.size(); i++) {
				if (lmax < fabs(ldata[i]) )
					lmax = fabs(ldata[i]);
				if (rmax < fabs(rdata[i]) )
					rmax = fabs(rdata[i]);
			}
			if (lmax < rmax)
				max = rmax;
			else
				max = lmax;
			for (int i = 0; i < ldata.size(); i++) {
				ldata[i] /= max;
				rdata[i] /= max;
			}
		}
	}
}

/*--------------------------------------------------------------------------
 * Set FileName
 */
void Wave::SetName(const std::string filename) {
	wavename = filename;
	wavename += ".wav";
}

/*--------------------------------------------------------------------------
 * Get Monoral Data
 */
void Wave::GetData(std::vector<double> *mono) {
	if (flag == false) {
		std::cerr << "Please Create or Input Wave." << std::endl;
	}
	else {
		if (channels == 1)
			*mono = monodata;
		else {
			std::cerr << "Stereoです." << std::endl;
		}
	}
}

/*--------------------------------------------------------------------------
 * Get Stereo Data
 */
void Wave::GetData( std::vector<double> *stereoL , std::vector<double> *stereoR ) {
	if (flag == false) {
		std::cerr << "Please Create or Input Wave." << std::endl;
	}
	else {
		if(channels == 2) {
			*stereoL = ldata;
			*stereoR = rdata;
		}
		else {
			std::cerr << "Monoralです." << std::endl;
		}
	}
}

/*--------------------------------------------------------------------------
 * Set Monoral Data
 */
void Wave::SetData(const std::vector<double> mono) {
	if (flag == false) {
		std::cerr << "Please Create or Input Wave." << std::endl;
	}
	else {
		ldata.clear();
		rdata.clear();
		monodata = mono;
		channels = 1;
		sizeOfData = monodata.size() * ( bitsPerSample / 8 );
	}
}

/*--------------------------------------------------------------------------
 * Set Stereo Data
 */
void Wave::SetData(const std::vector<double> stereoL , const std::vector<double> stereoR ) {
	if (flag == false) {
		std::cerr << "Please Create or Input Wave." << std::endl;
	}
	else {
		monodata.clear();
		ldata = stereoL;
		rdata = stereoR;
		channels = 2;
		sizeOfData = ldata.size() * 2 * ( bitsPerSample / 8 );
	}
}

/*--------------------------------------------------------------------------
 * Resampling Wave for Sinc
 */
void Wave::ResamplingSinc(const uint16_t resampling,const int sincLengthHarf) {
	if (flag == false) {
		std::cerr << "Please Create or Input Wave." << std::endl;
	}
	else {
		double pitch;
		int sincLength;
		double t;
		int offset;
		int n,m;
		std::vector<double> mono;
		std::vector<double> stereoL,stereoR;

		pitch = (double)samplesPerSec / (double)resampling;
		sincLength = sincLengthHarf*2;
		
		if ( channels == 1 ) {
			mono.resize( monodata.size() / pitch );

			for (n = 0; n < mono.size(); n++) {
				t = pitch * n;
				offset = (int)t;
				for (m = offset - (sincLength / 2); m <= offset + (sincLength / 2) ; m++) {
					if ( m >= 0 && m < monodata.size() ) {
						mono[n] += monodata[m] * Sinc(M_PI * ( t - m ) );
					}
				}
			}
			monodata = mono;
			samplesPerSec = resampling;
			sizeOfData = mono.size() * wavfhdr.wavfmt.blockAlign;
			SetHeader();
		}
		else if ( channels == 2 ) {
			stereoL.resize( ldata.size() / pitch);
			stereoR.resize( rdata.size() / pitch);

			for (n = 0; n < stereoL.size(); n++) {
				t = pitch * n;
				offset = (int)t;
				for (m = offset - (sincLength / 2); m <= offset + (sincLength / 2) ; m++) {
					if ( m >= 0 && m < ldata.size() ) {
						stereoL[n] += ldata[m] * Sinc(M_PI * ( offset - m ) );
						stereoR[n] += rdata[m] * Sinc(M_PI * ( offset - m ) );
					}
				}
			}
			ldata = stereoL;
			rdata = stereoR;
			samplesPerSec = resampling;
			sizeOfData = ldata.size() * wavfhdr.wavfmt.blockAlign;
			SetHeader();
		}
	}
}


