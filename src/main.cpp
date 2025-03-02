#include <chrono>
#include <cmath>
#include <complex>
#include <fstream>
#include <iostream>
#include <SDL2/SDL.h>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include <vector>

using namespace std;

string helpString = "Spectrograph - Terminal audio visualizer\n"
"Usage: spectrograph {-flags} <file>\n"
"CURRENTLY ONLY WORKS WITH RAW FILES!!!\n"
"Flags:\n"
"-a <size>: Size of audio buffer, in bytes (Default: 1024)\n"
"-b <size>: Size of DFT buffer, in bytes (Default: 1024)\n"
"-c <char>: Select the character that represents the spectrogram\n"
"-ch <channel>: Which channel you want to visualize (Default: 1)\n"
"-c1 <r> <g> <b>: Color of bottom of gradient\n"
"-c2 <r> <g> <b>: Color of top of gradient\n"
"-d: Shows debug info\n"
"-f: File name\n"
"-fc: Number of channels (Default: 1)\n"
"-fo: Specifies audio data format (s8|s16|s32|u8|u16, default: u8)\n"
"-fq <freq>: Selects file frequency (default: 44100 Hz)\n"
"-h: Shows this menu\n"
"-l: Loops audio indefinitely\n"
"-n: Shows hz and db\n"
"-s <div>: Static dB - Divides PSD by div\n"
"-v <float>: Sets volume of output audio (0.0-1.0)\n"
"-w <time>: Wait time in microseconds between frames\n";

SDL_AudioSpec audioSpec;

ifstream song;

double volume;

void audioMixer(void* ud, unsigned char* stream, int len)
{
    song.read((char*)stream, len);
    for(int i = 0; i < len; i++) stream[i] = (char)((double)stream[i]*volume);
}

int audioOpen(int frequency, SDL_AudioFormat format, int channels, int samples)
{
    SDL_AudioSpec as;

    as.freq = frequency;
    as.format = format;
    as.channels = 1;
    as.samples = samples;
    as.callback = audioMixer;

    if(SDL_OpenAudio(&as, &audioSpec) != 0) return 1;
    if(audioSpec.format != AUDIO_U8) return 1;

    SDL_PauseAudio(0);
    return 0;
}

int main(int argc, char* argv[])
{
    if(argc == 1)
    {
        cout << "Usage: spectrograph <file>" << endl;
        cout << "Type spectrograph -h for help" << endl;
        return 1;
    }

    char character = '#';
    bool debug = false;
    bool showNumbers = false;
    int waitTime = 1000000 / 144;
    unsigned bufferSize = 1024;
    unsigned audioBufferSize = 1024;

    SDL_AudioFormat audioFormat = AUDIO_U8;
    int numChannels = 1;
    int visChannel = 1;
    unsigned frequency = 44100;
    bool staticDB = false;
    int divisor;

    bool loop = false;

    int r1 = 0, g1 = 255, b1 = 0;
    int r2 = 255, g2 = 0, b2 = 0;

    bool useSongName = false;
    string songName;

    if(argc > 2)
    {
        for(int i = 1; i < argc;)
        {
            string argument = argv[i];

            if(argument == "-a")
            {
                if(i==argc-1)
                {
                    cout << "Malformed expression: -a" << endl;
                    return 1;
                }
                audioBufferSize = atoi(argv[i+1]);
                i += 2;
            }
            else if(argument == "-b") // Buffer size
            {
                if(i==argc-1)
                {
                    cout << "Malformed expression: -b" << endl;
                    return 1;
                }
                bufferSize = atoi(argv[i+1]);
                i += 2;
            }
            else if(argument == "-c")
            {
                if(i==argc-1)
                {
                    cout << "Malformed expression: -c" << endl;
                    return 1;
                }
                character = *(argv[i+1]);
                i += 2;
            }
            else if(argument == "-ch")
            {
                if(i==argc-1)
                {
                    cout << "Malformed expression: -c" << endl;
                    return 1;
                }
                visChannel = atoi(argv[i+1]);
                i += 2;
            }
            else if(argument == "-c1")
            {
                if(i>argc-4)
                {
                    cout << "Malformed expression: -c1" << endl;
                    return 1;
                }
                r1 = atoi(argv[i+1]);
                g1 = atoi(argv[i+2]);
                b1 = atoi(argv[i+3]);
                i += 4;
            }
            else if(argument == "-c2")
            {
                if(i>argc-4)
                {
                    cout << "Malformed expression: -c2" << endl;
                    return 1;
                }
                r2 = atoi(argv[i+1]);
                g2 = atoi(argv[i+2]);
                b2 = atoi(argv[i+3]);
                i += 4;
            }
            else if(argument == "-d") // Debug
            {
                debug = true;
                i++;
            }
            else if(argument == "-f")
            {
                if(i==argc-1)
                {
                    cout << "Malformed expression: -f" << endl;
                    return 1;
                }
                useSongName = true;
                songName = argv[i+1];
                i += 2;
            }
            else if(argument == "-fc")
            {
                if(i==argc-1)
                {
                    cout << "Malformed expression: -fc" << endl;
                    return 1;
                }
                numChannels = atoi(argv[i+1]);
                i += 2;
            }
            else if(argument == "-fo")
            {
                if(i==argc-1)
                {
                    cout << "Malformed expression: -fo" << endl;
                    return 1;
                }
                if(argv[i+1] == "s8") audioFormat = AUDIO_S8;
                if(argv[i+1] == "u8") audioFormat = AUDIO_U8;
                if(argv[i+1] == "s16") audioFormat = AUDIO_S16;
                if(argv[i+1] == "u16") audioFormat = AUDIO_U16;
                if(argv[i+1] == "s32") audioFormat = AUDIO_S32;
                i += 2;
            }
            else if(argument == "-fq")
            {
                if(i==argc-1)
                {
                    cout << "Malformed expression: -fq" << endl;
                    return 1;
                }
                frequency = atoi(argv[i+1]);
                i += 2;
            }
            else if(argument == "-h") // Help
            {
                cout << helpString;
                return 0;
            }
            else if(argument == "-l") // Loop
            {
                loop = true;
                i++;
            }
            else if(argument == "-n") // Hz && dB
            {
                showNumbers = true;
                i++;
            }
            else if(argument == "-s")
            {
                if(i==argc-1)
                {
                    cout << "Malformed expression: -s" << endl;
                    return 1;
                }
                staticDB = true;
                divisor = atoi(argv[i+1]);
                i += 2;
            }
            else if(argument == "-v")
            {
                if(i==argc-1)
                {
                    cout << "Malformed expression: -v" << endl;
                    return 1;
                }
                volume = atof(argv[i+1]);
                i += 2;
            }
            else if(argument == "-w") // Wait time
            {
                if(i==argc-1)
                {
                    cout << "Malformed expression: -w" << endl;
                    return 1;
                }
                waitTime = atoi(argv[i+1]);
                i += 2;
            }
            else if(i != argc-1)
            {
                cout << "Unknown argument: " << argv[i] << endl;
                return 1;
            }
        }
    }
    else
    {
        if(string(argv[1]) == "-h")
        {
            cout << helpString;
            return 0;
        }
        else if(string(argv[1]) == "-v")
        {
            cout << "Spectrograph v0.0.1" << endl;
            return 0;
        }
    }

    if(visChannel > numChannels)
    {
        cout << "Wrong number of channels: Wanted to visualize channel " << visChannel << ", but only " << numChannels << " specified!" << endl;
        return 1;
    }

    song = ifstream(useSongName?songName:argv[argc-1], ios::binary);

    if(!song.is_open())
    {
        cout << "Error reading " << (useSongName?songName:argv[argc-1]) << endl;
        return 1;
    }

    streampos begin, end;
    begin = song.tellg();
    song.seekg(0,song.end);
    end = song.tellg();
    unsigned songSize = (end - begin);

    struct winsize ws;
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        cout << "Could not determine terminal size." << endl;
        return 1;
    }

    if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        cout << "Couldn't initialize SDL." << endl;
        return 1;
    }
    SDL_AudioStream *stream = SDL_NewAudioStream(audioFormat, numChannels, frequency, AUDIO_F32, numChannels, 48000);
    if(stream == NULL)
    {
        cout << "Couldn't initialize stream: " << SDL_GetError() << endl;
        return 1;
    }

    if(audioOpen(frequency, audioFormat, numChannels, audioBufferSize) != 0)
    {
        cout << "Couldn't open audio device." << endl;
        return 1;
    }

    cout << "\x1b[2J";
    //cout << "\x1b[?25l";
    cout.flush();

    double time = 0;
    auto start = chrono::high_resolution_clock::now();
    while(1)
    {
        if((int)(time*frequency) + bufferSize > songSize)
        {
            if(!loop) break;
            start = chrono::high_resolution_clock::now();
            time = 0;
        }

        auto begin = chrono::high_resolution_clock::now();
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);

        cout << "\033[2J";
        
        vector<complex<double>> values;
        values.clear();

        song.seekg((unsigned)(time*frequency),song.beg);
        char buffer[bufferSize];
        song.read(buffer, bufferSize);

        if(SDL_AudioStreamPut(stream, buffer, bufferSize) == -1)
        {
            cout << "Error putting samples in stream: " << SDL_GetError() << endl;
            return 1;
        }

        float newBuffer[bufferSize*numChannels];
        if(SDL_AudioStreamGet(stream, newBuffer, sizeof(newBuffer)) == -1)
        {
            cout << "Error parsing data from internal buffer: " << SDL_GetError() << endl;
            return 1;
        }
        
        // Fourier Transform - TODO: DFT -> FFT
        for(unsigned k = 0; k < ws.ws_col; k++)
        {
            complex sum(0.0, 0.0);
            for(unsigned j = visChannel-1; j < bufferSize; j+=numChannels)
            {
                sum += ((double)newBuffer[j] * 256) * complex(cos(M_PI*j*k/bufferSize),-sin(M_PI*j*k/bufferSize));
            }

            values.push_back(sum);
        }

        complex maxValue = complex(-1000000000.0,0.0);
        if(!staticDB) for(int i = 0; i < values.size(); i++) if(values[i].real() > maxValue.real()) maxValue = values[i];

        for(int i = 0; i < ws.ws_col; i++)
        {
            int count;
            if(staticDB) count = max(min((int)(pow((10*log(values[i].real()))/(10*log(divisor)),5)*ws.ws_row),(int)ws.ws_row),0);
            else count = max(min((int)(values[i].real()/maxValue.real()*ws.ws_row),(int)ws.ws_row),0);
            for(int k = 0; k < count; k++)
            {
                if((showNumbers && i > 5 && k != 0) || !showNumbers)
                {
                    double t = max(min(sqrt((double)k/ws.ws_row),1.0),0.0);
                    unsigned r = max(min((int)(r1 + t*(r2-r1)),255),0);
                    unsigned g = max(min((int)(g1 + t*(g2-g1)),255),0);
                    unsigned b = max(min((int)(b1 + t*(b2-b1)),255),0);
                    cout << "\x1b[38;2;" << r << ";" << g << ";" << b << "m";
                    cout << "\x1b[" << (ws.ws_row-k) << ";" << i << "H" << character;
                }
            }
        }

        int timeTaken = (int)chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - begin).count();

        if(showNumbers)
        {
            cout << "\x1b[38;5;15m";
            cout << "\x1b[999;1H";
            // Hz
            double dt = 1.0 / frequency * bufferSize;
            string hz;
            for(int i = 0; i < ws.ws_col-1; i++)
            {
                if(i%10 == 0)
                {
                    if(i == 0) cout << "+";
                    else
                    {
                        double currentFreq = frequency * (i+5) / bufferSize;
                        if(currentFreq < 1000) hz = to_string((int)currentFreq) + "Hz";
                        else
                        {
                            hz = to_string((int)(currentFreq/1000)) + "." + to_string((int)(((int)currentFreq%1000)/100)) + "kHz";
                        }
                        cout << hz[0];
                    }
                }
                else
                {
                    if(i%10 < hz.length()) cout << hz[i%10];
                    else cout << "-";
                }
            }

            // dB
            string dB;
            for(int i = 0; i < ws.ws_row; i++)
            {
                cout << "\x1b[" << ws.ws_row-i << ";1H";
                if(i == 0) cout << "+";
                else if(i%3 == 0)
                {
                    if(staticDB) dB = to_string((int)(10*log(divisor)*(pow((double)i/ws.ws_row,1.0/5)))) + "dB";
                    else dB = to_string((int)(10*log(maxValue.real()/(ws.ws_row-i)))) + "dB";
                    cout << dB;
                }
                else cout << "|";
            }

            if(debug)
            {
                cout << "\x1b[38;5;15m";
                cout << "\x1b[1;1H" << "Delta: " << timeTaken << "us";
                cout << "\x1b[2;1H" << "Time: " << time << "s";
                cout << "\x1b[3;1H" << "Size: " << songSize;
            }
        }
        
        cout << "\x1b[999;999H";
        cout.flush();
        time = chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - start).count() / 1e6;
        usleep(max(waitTime - timeTaken,(int)0));
    }

    usleep(100000);

    cout << "\x1b[2J";
    cout << "\x1b[38;5;15m";
    cout << "\x1b[H";
    cout.flush();

    return 0;
}