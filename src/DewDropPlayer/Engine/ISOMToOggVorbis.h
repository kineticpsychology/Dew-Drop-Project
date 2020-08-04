/*******************************************************************************
*                                                                              *
* COPYRIGHT (C) Aug, 2020 | Polash Majumdar                                    *
*                                                                              *
* This file is part of the Dew Drop Player project. The file content comes     *
* "as is" without any warranty of definitive functionality. The author of the  *
* code is not to be held liable for any improper functionality, broken code,   *
* bug and otherwise unspecified error that might cause damage or might break   *
* the application where this code is imported. In accordance with the          *
* Zero-Clause BSD licence model of the Dew Drop project, the end-user/adopter  *
* of this code is hereby granted the right to use, copy, modify, and/or        *
* distribute this code with/without keeping this copyright notice.             *
*                                                                              *
* TL;DR - It's a free world. All of us should give back to the world that we   *
*         get so much from. I have tried doing my part by making this code     *
*         free (as in free beer). Have fun. Just don't vandalize the code      *
*         or morph it into a malicious one.                                    *
*                                                                              *
*******************************************************************************/

#ifndef _ISOMTOOGGVORBIS_H_
#define _ISOMTOOGGVORBIS_H_

// This is a very specific class. You can think of it as somewhat same as that of ALACLib.
// It does not not have an existence of its own. This means the constructor parameter
// cannot be provided apart from anyone other than the Bento4 parser.

// Most of the formatting/reverse engineering/packing approach
// has been taken from the official ogg packet specification:
// https://xiph.org/vorbis/doc/framing.html
// The CRC logic is "borrowed" from the encoder example provided with libvorbis
#include "IAudioEngine.h"
#include <inttypes.h>

typedef class ISOMTOOGGVORBIS
{
    private:
        template <class T> void SafeRelease(T **ppT)
        {
            if (*ppT)
            {
                (*ppT)->Release();
                *ppT = NULL;
            }
        }

        template <class T> void SafeFree(T **ppT)
        {
            if (*ppT)
            {
                LocalFree(*ppT);
                *ppT = NULL;
            }
        }

        static const DWORD  _OGGS_SIGNATURE = 0x5367674F;
        static const DWORD  _MAX_VORBIS_BLOCK_SIZE = 0xFE00; // (0xFF * 0xFF): Maximum size that can be represented by an index of 0xFF bytes long, with each byte representing 0xFF
        static const DWORD  _MAX_SEGMENT_TABLE_SIZE = 0xFF; // This is the maximum segment table size (representable by 1 byte)
        DWORD               _dwSerial = 0, _dwPageSequence = 0;
        DWORD               _dwSegmentTableSize = 0, _dwTotalVorbisBlockSize = 0; // We just need the sizes of the index tables and the actual vorbis data blocks. Headers are 0x1B long anyways
        unsigned long long  _u64GranulePos = 0x00;
        uint32_t            _CRCLookup[256];

        // Instead of creating dynamic memories/streams, we'll allocate fixed mem buffers
        // with anticipated max values. This will reduce the overhead of mem allocation
        // for each packet. This will increase the performance and stability.
        BYTE                _btOggHeader[0x1B]; // Holds the header (which is alwyas 0x1B long)
        BYTE                _btVorbisSegmentTable[_MAX_SEGMENT_TABLE_SIZE]; // The segment table index. This can be at most 0xFF bytes long, since there is only 1 byte to represent index table length in the page header (offset 0x1B)
        BYTE                _btVorbisBlockData[_MAX_VORBIS_BLOCK_SIZE]; // The actual vorbis data block. This can at max be 0xFF x 0xFF bytes long.
        BYTE                _btOggPage[0x1B + _MAX_SEGMENT_TABLE_SIZE + _MAX_VORBIS_BLOCK_SIZE]; // On page with max capacity. Needed to calculate CRC and dump into stream in a single go.

        IStream             *_pFullAudioTrack = NULL; // Main audio track to deliver back to the ISOMEngine
        BOOL                _bInputValidated = FALSE;
        DWORD               _dwSampleRate = 0;

        virtual void        _GenerateLookupTable();
        virtual DWORD       _GetCRC(LPBYTE lpData, DWORD dwDataSize);
        virtual void        _EncapsulateVorbisToOgg(BYTE btPageType = 0);

    public:
                            ISOMTOOGGVORBIS(const LPBYTE pHeaderAndCodebook, DWORD dataBlockSize);
        virtual BOOL        AddVorbisBlock(const LPBYTE pVorbisAudioBlock, DWORD dataBlockSize, unsigned long long u64GranulePos, BOOL lastBlock = FALSE);
        virtual BOOL        GetFullAudioTrack(LPBYTE *ppTrackData, LPDWORD pTrackLength);
        virtual              ~ISOMTOOGGVORBIS();

} ISOMTOOGGVORBIS, *LPISOMTOOGGVORBIS;

#endif
