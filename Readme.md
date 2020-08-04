### Dew Drop Player

------



#### About the project

------

The Dew Drop Player is a Windows based audio player, that is capable of playing several audio formats including container formats like (MP4, OGG). The primary objective of this player is to be a portable player with the minimal attributes:

- Dependencies on frameworks like (.NET Framework, JVM)
- Memory footprint
- Executable size
- Installation overhead

This has been achieved by implementing the following:

| **Approach**                          | **Reason**                                                   |
| :------------------------------------ | ------------------------------------------------------------ |
| Win32 API                             | Direct system calls to reduce framework overheads            |
| Mostly C, with a slight flavor of C++ | Remove any dependency with MS VC++ redistributable           |
| **/MT** switch for exe compilation    | Remove any dependency with MS VC++ redistributable           |
| Static libs for **all** libraries     | Remove dependency on library DLLs                            |
| Command line and mutex                | Support drag drop but no registry entry                      |
| ***waveOutXXX*** API                  | Remove dependency on later Windows audio APIs (DirectSound, XAudio, WASAPI, ASIO) |


The culmination of the above approaches guarantee that Dew Drop Player will run on Windows 7 and above, without needing to install audio drivers, codecs, frameworks or needing to install the player itself. Below are the features of the player.

| **Feature**                                                  |    **Supported**    |
| ------------------------------------------------------------ | :-----------------: |
| Regular multimedia controls (Play, Pause, Stop, Previous, Next, Seek, Volume) |          ✓          |
| Extended multimedia controls (Balance, Spectrum Analyzer, Graphic Equalizer) |          ✗          |
| Global multimedia-hotkey based action (Play/Pause/Stop/Next/Previous) |          ✓          |
| Playlist reading and exporting                               | ✓ (PLS, M3U & M3U8) |
| Skinning/Theming                                             |          ✓          |
| Minimizing to tray                                           |          ✓          |



Supported Audio Codec formats (and containers):

| **Format**                                                   | **Support** | **Bit support** | **Remarks**                                                  |
| ------------------------------------------------------------ | :---------: | --------------- | ------------------------------------------------------------ |
| AAC (Raw **.aac** files)                                     |      ✓      | 16              | Mono and stereo channels only. Supported profiles: LC, LTP, Main, HA AAC, HEv2 AAC |
| A/52 (**.ac3** files)                                        |      ✓      | 16              | Mono and stereo channels only                                |
| AIFF (**.aiff**, **.aif** files)                             |      ✓      | 16              |                                                              |
| Monkey's Audio (**.ape** files)                              |      ✓      | 16              |                                                              |
| CD Audio (**.cda** files, Audio CDs)                         |      ✓      | 16              | Non-encrypted, linear audio CDs only                         |
| FLAC Lossless audio (**.flac** files, ogg wrapped flac files with **.ogg**/**.oga** extension) |      ✓      | 16, 24          |                                                              |
| Midi audio (**.mid**, **.midi** files)                       |      ✓      | -               | Might not support Volume/Seeking on some systems             |
| Musepack (**.mpc** files)                                    |      ✓      | 16              |                                                              |
| MPEG Audio Layers 2 &3 (**.mp2**, **.mp3** files)            |      ✓      | 16              |                                                              |
| Opus Audio (**.opus** files, ogg wrapped opus files with **.ogg**/**.oga** extension) |      ✓      | 16              |                                                              |
| PCM WAV Audio (**.wav** files)                               |      ✓      | 16, 24          | Supports only unsigned 16/24 bit uncompressed audio          |
| Windows Media Audio (**.asf**, **.wma** files)               |      ✓      | 16              |                                                              |
| Wavepack Audio (**.wv** files)                               |      ✓      | 8, 16           | Does not support 20, 24, 32 bit audios                       |
| Vorbis (vorbis encoded **.ogg**, **.oga** files)             |      ✓      | 16              |                                                              |
| ISO/IEC audio container (**.3gp**, **.m4a**, **.mp4**, **.m4v** files) |      ✓      | 16              | Supports AAC, Apple's Lossless audio (ALAC), A/52 (AC3), Vorbis, MP2, MP3 audio codec streams wrapped in MP4/3gp containers. Supports only the first audio track in a multi-audio track mp4/m4a container. |
| Apple's Lossless Audio (ALAC in **.mp4**/**.m4a** container) |      ✓      | 16              |                                                              |



#### Getting Started

------

This project is dependent on multiple static libraries. Most libraries are from the wonderful people/teams who made their projects available to all for use. Apart from these external projects, this project also relies on some inbuilt and some customized libraries. Below are the details of all the projects involved. For reusability, all these static libraries are pre-built so that they can be plugged directly into the source and you can build the player without the hassle of building these libraries individually. Because of size constraints, all the libraries are high-compressed using the splendid [7-zip](https://www.7-zip.org/) archiving program. You can unzip to their local locations to get the actual ***.lib*** files.

| **Codec/Container**                                          | **Module Type**         | **Usage Notes**                 |
| ------------------------------------------------------------ | ----------------------- | ------------------------------- |
| [libmpg123](https://www.mpg123.org/index.shtml) (1.25.10)    | External Static Library | Build and use as-is             |
| [libogg](https://www.xiph.org/) (1.3.3)                      | External Static Library | Build and use as-is             |
| [libvorbis](https://www.xiph.org/)/[vorbisfile](https://www.xiph.org/) (1.3.6) | External Static Library | Build and use as-is             |
| [libflac](https://xiph.org/flac/) (1.3.3)                    | External Static Library | Build and use as-is             |
| [opusfile](http://opus-codec.org/) (0.9)                     | External Static Library | Build and use as-is             |
| [MACLib](https://www.monkeysaudio.com/) (5.21)               | External Static Library | Build and use as-is             |
| [libfaad](https://www.audiocoding.com/faad2.html) (2.8.8)    | External Static Library | Build and use as-is             |
| [ALACLib](https://github.com/macosforge/alac)                | Custom library          | Build and use as-is             |
| [liba52](http://liba52.sourceforge.net/) (0.7.4)             | External Static Library | Build and use as-is             |
| [AIFFLib](http://aifftools.sourceforge.net/libaiff/) (5.0)   | Custom library          | Build and use as-is             |
| [libmpc](https://www.musepack.net/) (r475)                   | External Static Library | Build and use as-is             |
| [libwavpack](http://www.wavpack.com/downloads.html) (5.1.0)  | External Static Library | Customized with UNICODE support |
| [bento4](https://www.bento4.com/) (1.5.1-628)                | External Static Library | Customized with UNICODE support |
| [taglib](https://taglib.org/) (1.11.1)                       | External Static Library | Build and use as-is             |



<u>Other (re)sources:</u>

| **Resource**                                                 | **Purpose**                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [Gary Kessler's file signature repository](https://www.garykessler.net/library/file_sigs.html) | A vast collection of file signatures                         |
| [Audio CD parsing project](https://www.codeproject.com/Articles/15725/Tutorial-on-reading-Audio-CDs) | A neat project on parsing and extracting WAV data out of audio CDs. |
| [ADTS Header structure](https://wiki.multimedia.cx/index.php/ADTS) | Parsing raw AAC files and reading the ADTS headers for seeking |
| [ISO Base media specifications](https://tools.ietf.org/html/rfc6381#section-3.3) | Official specs for ISO Base Media File Format                |



#### Prerequisites

------

You’ll need:

To build the core player & themer, you'll need:

* Visual Studio 2017

To build all the libraries individually, you'll need:

* YASM
* NASM
* CMake

Libraries like libmpg and libflac will need the assemblers. CMake will mostly be needed for building the taglib library.



#### Building Dew Drop

------

As mentioned earlier, if you just want to build the player, you can simply download the pre-compiled static libraries and run the ***Visual Studio 2017*** compatible solution to build the application. Please note that if you use a newer version of Visual Studio, you *might* run into issues with static library compatibility.

However, if you intend to completely build all the libraries yourself, then you can download the remaining libraries from their individual links provided in the '***Getting Started***' section.

Please make sure you strictly follow these instructions for a comfortable build:

* You MUST build each library with the **/MDd** switch for Debug build and **/MT** switch for Release build. The entire project has been laced with this combination. If you deviate with any intermediate project, you might run into issues when building the main Dew Drop project.

* Place the binary executables of YASM and NASM into the **%WINDIR%** (most likely *C:\windows*), to ensure you do not run into the unrecognized program error. Otherwise you might have to alter the custom build instructions for those projects to point the assembler binaries to their right paths.
* The suite of all the static libraries is divided into two builds – debug and release. The Dew Drop Project directory locations are also mapped accordingly. The code also refers to those paths with the **#define-_DEBUG-#pragma** comment combinations.
* By default, the Dew Drop project will build both the player as well as the themer. You can choose to selectively build either or both.
* Please note that the **DEW_THEMER_MODE** switch MUST be defined in the pre-processor directive for the Dew Drop Themer. Otherwise the Dew Drop Themer build will fail.
* Please read the resource headers, resource files and the relative paths of the respective resources **very carefully** before attempting to make any changes to them. Both the projects share a big chunk of code as well as the resources (like the Dew Drop Font).



#### Current Version

------

Dew Drop Player

Current Version: **1.3.0.0**

Build: **2363**

Dew Drop Themer

Current Version: **0.7.0.0**

Build: **216**

A note about build:

This is just a request to try and follow the release numbering configuration as explained below. However, you are free to adhere or ignore it.

* **Dew Drop Player**: Please subtract **February 14, 2014** from the date on which you are building this application. The build value should be the number of days in the difference. For example, if you are building this application on August 15, 2021, the build number should be 2739 (no. of days between Aug 15, 2021 and Feb 14, 2014).
* **Dew Drop Themer**: Please subtract **January 01, 2020** from the date on which you are building this application. The build value should be the number of days in the difference. For example, if you are building this application on August 15, 2021, the build number should be 592 (no. of days between Aug 15, 2021 and Jan 01, 2020).



#### Contributing

------

While the player could use quite a bit of improvements and enhancements, please keep in mind the basic philosophies of this project:

* Please follow Hungarian notation. This is a Win32 application after all and this notation deserves some recognition here. An acceptable variable name would be ***nCurrStreamPos*** for a ***UINT***. ***currentStreamPos*** is <u>unacceptable</u>.
* Please avoid modern C++ (***C++11***, ***C++14***, ***C++17***, ***C++20***). Although taglib uses this part, it is best avoided to remove any accidental dependency on MS VC++ redistributable.
* Try using WinAPI calls wherever possible, and avoid C standard library calls. For example, **LocalAlloc** or **HeapAlloc** is preferable over **malloc**. This is a windows-specific application anyways. So, we need not go with platform-independent C functions.
* Do **NOT** create any external file dependency whatsoever. This is a strict no-no. The whole purpose of the player is to be portable. So the project MUST always output only two files - '***DewDropPlayer.exe***' and '***DewDropThemer.exe***'. It is advisable to push all your resources into the respective .rc files as ***RC_DATA*** and then do a ***LoadResource***/***LockResource*** API combo to get hold of the data from within the application. 
* Try creating any window using only ***CreateWindow*** or ***CreateWindowEx*** APIs. Do not use any resource dialog. The idea here is to prevent any UI change using resource hackers. One can argue that hackers still can decompile and alter the UI. Yes they can; but that count of users are far less than the folks who would like to use a resource hacker on the application.

Here are the items that could be added to the player:

* .webm support (only audio)
* .mkv support (only audio)
* Graphic Equalizer
* Spectrum Analyzer (using FFTW library)
* L-R Balance
* Volume control fix for MIDI (.mid) files



#### Author

------

Polash Majumdar (***polash.majumdar (at) gmail.com***)



#### License

------

This project is licensed under Zero-clause BSD. Please see the project license for more details.



#### Acknowledgements

------

***Thanks / ধন্যবাদ...***

* **Sonali Majumdar** (**সোনালী মজুমদার** - *thank you for putting up with all my eccentricities and bearing with my long hours of mental absence*)
* **Dipankar Ghosh** (**দীপঙ্কর ঘোষ** - *thank you for the brilliant inputs with application behaviors*)
* **Sumanta Guha** (**সুমন্ত গুহ** - *thank you for the brilliant icon design ideas*)
* All you benevolent geniuses and code stalwarts who graced this virtual world with your wisdom of code and contributions!