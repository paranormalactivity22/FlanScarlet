#include <iostream>
#include <Windows.h>
#include <array>
#pragma comment(lib, "winmm.lib")

#define DBG_PRINT(x) std::cout << "[~~" << x << "] OKAY\n";

CONST INT SAMPLE_RATE = 19900;
CONST INT SAMPLE_SECONDS = 40;

double generateSample(int t) {
    auto r = t / 1.04;
    auto squ = [](int x) { return x & 32; };
    auto n = [](int x, int t) { return t * pow(2, (x + 5.5) / 12); };

    const std::array<int, 64> bytebeat_table1{ {
        0, 0, -1, -1, 7, 7, -1, -1, 2, 2, -1, -1, 7, 7, -1, -1,
        3, 3, -1, -1, 5, 5, 7, 7, 5, 5, -1, -1, 9, 9, -1, -1,
        12, 12, 7, 7, 14, 14, 15, 15, 14, 14, 15, 14, 12, 12, 10, 10,
        7, 7, 10, -1, 5, 5, 7, -1, 3, 3, 3, 3, 3, 3, 3, 3
    } };

    const std::array<int, 64> bytebeat_table2{ {
        0, 0, -1, -1, 7, 7, -1, -1, 2, 2, -1, -1, 7, 7, -1, -1,
        4, 4, -1, -1, 6, 6, 7, 7, 6, 6, -1, -1, 9, 9, -1, -1,
        12, 12, 7, 7, 14, 14, 15+1, 15+1, 14, 14, 15+1, 14, 12, 12, 9+1, 9+1,
        7, 7, 9+1, -1, 6-1, 6-1, 7, -1, 4-1, 4 - 1, 4 - 1, 4 - 1, 4 - 1, 4 - 1, 4 - 1, 4 - 1
    } };

    return squ(n(bytebeat_table1[63 & (int)r >> 12], t)) * 2 +
        squ(n(bytebeat_table2[63 & (int)r >> 12], t) * 0.8) * 0.60606060606;
}


int main() {
    HWAVEOUT hWaveOut;
    WAVEFORMATEX waveFormat;
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nChannels = 1;
    waveFormat.nSamplesPerSec = SAMPLE_RATE;
    waveFormat.wBitsPerSample = 16;
    waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
    waveFormat.cbSize = 0;

    MMRESULT result = waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveFormat, NULL, 0, 0);
    if (result != MMSYSERR_NOERROR) {
        return 0;
    }
    DBG_PRINT("waveOutOpen");

    char buffer[SAMPLE_RATE * SAMPLE_SECONDS];
    memset(buffer, 0, sizeof buffer);
    for (DWORD t = 0; t < sizeof(buffer); t += 1)
    {
        buffer[t] = static_cast<unsigned char>(generateSample(t));
    }
    WAVEHDR waveHeader;
    waveHeader.lpData = buffer;
    waveHeader.dwBufferLength = sizeof(buffer);
    waveHeader.dwFlags = 0;
    waveHeader.dwLoops = SND_ASYNC | SND_LOOP;

    result = waveOutPrepareHeader(hWaveOut, &waveHeader, sizeof(WAVEHDR));
    if (result != MMSYSERR_NOERROR) {
        waveOutClose(hWaveOut);
        delete[] waveHeader.lpData;
        return 0;
    }
    DBG_PRINT("waveOutPrepareHeader");

    result = waveOutWrite(hWaveOut, &waveHeader, sizeof(WAVEHDR));
    if (result != MMSYSERR_NOERROR) {
        waveOutUnprepareHeader(hWaveOut, &waveHeader, sizeof(WAVEHDR));
        waveOutClose(hWaveOut);
        delete[] waveHeader.lpData;
        return 0;
    }
    DBG_PRINT("waveOutWrite");

    waveOutUnprepareHeader(hWaveOut, &waveHeader, sizeof(WAVEHDR));
    DBG_PRINT("waveOutUnprepareHeader");
    waveOutClose(hWaveOut);
    DBG_PRINT("waveOutClose");

    Sleep(SAMPLE_SECONDS * 500);
    return 0;
}