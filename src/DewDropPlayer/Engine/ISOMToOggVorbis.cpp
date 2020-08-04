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

#include "ISOMToOggVorbis.h"

// Taken straight from the framing.c file (libogg project).
// We could've gone with a global static here, but then
// we are trying to box everything into classes.
// Just dump the Polynomials into out non-static member
void ISOMTOOGGVORBIS::_GenerateLookupTable()
{
    const uint32_t CRCLookup[256] = {
        0x00000000,0x04c11db7,0x09823b6e,0x0d4326d9,
        0x130476dc,0x17c56b6b,0x1a864db2,0x1e475005,
        0x2608edb8,0x22c9f00f,0x2f8ad6d6,0x2b4bcb61,
        0x350c9b64,0x31cd86d3,0x3c8ea00a,0x384fbdbd,
        0x4c11db70,0x48d0c6c7,0x4593e01e,0x4152fda9,
        0x5f15adac,0x5bd4b01b,0x569796c2,0x52568b75,
        0x6a1936c8,0x6ed82b7f,0x639b0da6,0x675a1011,
        0x791d4014,0x7ddc5da3,0x709f7b7a,0x745e66cd,
        0x9823b6e0,0x9ce2ab57,0x91a18d8e,0x95609039,
        0x8b27c03c,0x8fe6dd8b,0x82a5fb52,0x8664e6e5,
        0xbe2b5b58,0xbaea46ef,0xb7a96036,0xb3687d81,
        0xad2f2d84,0xa9ee3033,0xa4ad16ea,0xa06c0b5d,
        0xd4326d90,0xd0f37027,0xddb056fe,0xd9714b49,
        0xc7361b4c,0xc3f706fb,0xceb42022,0xca753d95,
        0xf23a8028,0xf6fb9d9f,0xfbb8bb46,0xff79a6f1,
        0xe13ef6f4,0xe5ffeb43,0xe8bccd9a,0xec7dd02d,
        0x34867077,0x30476dc0,0x3d044b19,0x39c556ae,
        0x278206ab,0x23431b1c,0x2e003dc5,0x2ac12072,
        0x128e9dcf,0x164f8078,0x1b0ca6a1,0x1fcdbb16,
        0x018aeb13,0x054bf6a4,0x0808d07d,0x0cc9cdca,
        0x7897ab07,0x7c56b6b0,0x71159069,0x75d48dde,
        0x6b93dddb,0x6f52c06c,0x6211e6b5,0x66d0fb02,
        0x5e9f46bf,0x5a5e5b08,0x571d7dd1,0x53dc6066,
        0x4d9b3063,0x495a2dd4,0x44190b0d,0x40d816ba,
        0xaca5c697,0xa864db20,0xa527fdf9,0xa1e6e04e,
        0xbfa1b04b,0xbb60adfc,0xb6238b25,0xb2e29692,
        0x8aad2b2f,0x8e6c3698,0x832f1041,0x87ee0df6,
        0x99a95df3,0x9d684044,0x902b669d,0x94ea7b2a,
        0xe0b41de7,0xe4750050,0xe9362689,0xedf73b3e,
        0xf3b06b3b,0xf771768c,0xfa325055,0xfef34de2,
        0xc6bcf05f,0xc27dede8,0xcf3ecb31,0xcbffd686,
        0xd5b88683,0xd1799b34,0xdc3abded,0xd8fba05a,
        0x690ce0ee,0x6dcdfd59,0x608edb80,0x644fc637,
        0x7a089632,0x7ec98b85,0x738aad5c,0x774bb0eb,
        0x4f040d56,0x4bc510e1,0x46863638,0x42472b8f,
        0x5c007b8a,0x58c1663d,0x558240e4,0x51435d53,
        0x251d3b9e,0x21dc2629,0x2c9f00f0,0x285e1d47,
        0x36194d42,0x32d850f5,0x3f9b762c,0x3b5a6b9b,
        0x0315d626,0x07d4cb91,0x0a97ed48,0x0e56f0ff,
        0x1011a0fa,0x14d0bd4d,0x19939b94,0x1d528623,
        0xf12f560e,0xf5ee4bb9,0xf8ad6d60,0xfc6c70d7,
        0xe22b20d2,0xe6ea3d65,0xeba91bbc,0xef68060b,
        0xd727bbb6,0xd3e6a601,0xdea580d8,0xda649d6f,
        0xc423cd6a,0xc0e2d0dd,0xcda1f604,0xc960ebb3,
        0xbd3e8d7e,0xb9ff90c9,0xb4bcb610,0xb07daba7,
        0xae3afba2,0xaafbe615,0xa7b8c0cc,0xa379dd7b,
        0x9b3660c6,0x9ff77d71,0x92b45ba8,0x9675461f,
        0x8832161a,0x8cf30bad,0x81b02d74,0x857130c3,
        0x5d8a9099,0x594b8d2e,0x5408abf7,0x50c9b640,
        0x4e8ee645,0x4a4ffbf2,0x470cdd2b,0x43cdc09c,
        0x7b827d21,0x7f436096,0x7200464f,0x76c15bf8,
        0x68860bfd,0x6c47164a,0x61043093,0x65c52d24,
        0x119b4be9,0x155a565e,0x18197087,0x1cd86d30,
        0x029f3d35,0x065e2082,0x0b1d065b,0x0fdc1bec,
        0x3793a651,0x3352bbe6,0x3e119d3f,0x3ad08088,
        0x2497d08d,0x2056cd3a,0x2d15ebe3,0x29d4f654,
        0xc5a92679,0xc1683bce,0xcc2b1d17,0xc8ea00a0,
        0xd6ad50a5,0xd26c4d12,0xdf2f6bcb,0xdbee767c,
        0xe3a1cbc1,0xe760d676,0xea23f0af,0xeee2ed18,
        0xf0a5bd1d,0xf464a0aa,0xf9278673,0xfde69bc4,
        0x89b8fd09,0x8d79e0be,0x803ac667,0x84fbdbd0,
        0x9abc8bd5,0x9e7d9662,0x933eb0bb,0x97ffad0c,
        0xafb010b1,0xab710d06,0xa6322bdf,0xa2f33668,
        0xbcb4666d,0xb8757bda,0xb5365d03,0xb1f740b4 };

    CopyMemory(_CRCLookup, CRCLookup, 256 * sizeof(uint32_t));
    return;
}

// Loop through the byte array and calculate the CRC from the Polynomial table
// Again, this logic is "borrowed" straight from the framing.c file (libogg project)
DWORD ISOMTOOGGVORBIS::_GetCRC(LPBYTE lpData, DWORD dwDataSize)
{
    uint32_t dwCRC = 0;

    for (UINT nIndex = 0; nIndex < dwDataSize; nIndex++)
        dwCRC = (dwCRC << 8) ^ _CRCLookup[((dwCRC >> 24) & 0xFF) ^ lpData[nIndex]];
    return dwCRC;
}

// This function has four basic purposes:
// 1. Set the page type (b_o_s/intermittent/e_o_s) and collect everything into a packet
// 2. Calculate the CRC of the packet & update it back to the memory block
// 3. Write out the memory block into the main audio stream
// 4. Update/reset the trackers
void ISOMTOOGGVORBIS::_EncapsulateVorbisToOgg(BYTE btPageType)
{
    DWORD               dwGen = 0, dwVal = 0, dwPageSize = 0;

    _btOggHeader[0x05] = btPageType; // typically 0, but will be 0x02 for e_o_s or 0x04 for b_o_s
    CopyMemory(&(_btOggHeader[0x12]), &_dwPageSequence, 0x04); // 0x12 - 0x15 - 0-based Page Sequence
    CopyMemory(&(_btOggHeader[0x06]), &_u64GranulePos, 0x08); // 0x06 - 0x0D - Granule Position.
    _btOggHeader[0x1A] = (BYTE)_dwSegmentTableSize; // Segment table index size

    // Dump the 3-block stuff into the page container mem block:
    CopyMemory(&(_btOggPage[0x00]), _btOggHeader, 0x1B); // This is fixed. Header is always 0x1B bytes
    CopyMemory(&(_btOggPage[0x1B]), _btVorbisSegmentTable, _dwSegmentTableSize); // Segment table index
    CopyMemory(&(_btOggPage[0x1B + _dwSegmentTableSize]), _btVorbisBlockData, _dwTotalVorbisBlockSize);

    dwPageSize = 0x1B + _dwSegmentTableSize + _dwTotalVorbisBlockSize;
    dwVal = this->_GetCRC(_btOggPage, dwPageSize);
    CopyMemory(&(_btOggPage[0x16]), &dwVal, 0x04); // Basically, _btOggHeader & _btOggPage are the same till 0x1B btes

    // Update the trackers
    _dwSegmentTableSize = 0;
    _dwTotalVorbisBlockSize = 0;
    _u64GranulePos = 0;
    _dwPageSequence++;

    // Just write the stuff to the main stream.
    _pFullAudioTrack->Write(_btOggPage, dwPageSize, &dwPageSize);

    return;
}

// Constructor. Prepare the initial 2 pages of Ogg
// 1.  1st (b_o_s) page, containing audio properties info (sample rate, bitrate, channels etc). This is 0x1E bytes long
// 2a. 1st part of second page that contains the encoder info (like lavf version)
// 2b. 2nd part of second page that contains the vorbis codebook
// Once these are constructed, we will start pushing true vorbis audio data from 3rd page onwards
ISOMTOOGGVORBIS::ISOMTOOGGVORBIS(const LPBYTE pHeaderAndCodebook, DWORD dataBlockSize)
{
    BYTE                btTotalBlocks = 0;
    BYTE                btInfoBlockSize = 0, btCommentBlockSize = 0;
    DWORD               dwGen, dwCodebookBlockSize, dwIndex;
    //unsigned long long  u64GranulePos = 0;

    // Sanity check.
    if (!pHeaderAndCodebook || dataBlockSize < 0) return;

    // The data block consists typically 3 blocks (contiguous) - Audio sample/bitrate/channel, Encoder and Codebook
    // byte #0: gives the partition count (typically 0x02, indicating 3 blocks of data)
    // byte #1: size of the 1st block
    // byte #2: size of the 2nd block
    // byte #3: Start of 1st block (if 3 blocks). Otherwise, size of 3rd block (for 4 total blocks) and so on...
    btTotalBlocks = pHeaderAndCodebook[0];
    btInfoBlockSize = pHeaderAndCodebook[1];
    btCommentBlockSize = pHeaderAndCodebook[2];
    // The input block MUST contain at least 3 blocks as illustrated above
    if (btTotalBlocks < 2) return;
    // The Info block MUST always be 0x1E
    if (btInfoBlockSize != 0x1E) return;

    if (btTotalBlocks >= 3)
        dwCodebookBlockSize = pHeaderAndCodebook[3]; // codebook must lie within 2nd and 3rd partition
    else
        dwCodebookBlockSize = (dataBlockSize - (btInfoBlockSize + btCommentBlockSize) - 0x03); // codebook lies within 2nd partition and the end of data

    // Some basic initialization
    _dwSerial = 0xED0D9C09;   //'099C0DED', i.e. 'Ogg Coded' in little endian :) - Perma serial. We need not generate unnecessary randomized serial.
    this->_GenerateLookupTable();

    // Common Ogg Page entries that will never change
    CopyMemory(&(_btOggHeader[0x00]), &_OGGS_SIGNATURE, 0x04); // 'OggS' signature
    _btOggHeader[0x04] = 0x00; // Version: Always zero.
    CopyMemory(&(_btOggHeader[0x0E]), &_dwSerial, 0x04); // Ogg Serial (0x0E - 0x011)

    // Now, create all the necessary streams:
    _pFullAudioTrack = SHCreateMemStream(NULL, 0);

    // First Ogg Page ----------------------------------------------------------
    // HEADER ------------------------------------------------------------------
    _u64GranulePos = 0; // 0x06 - 0x0D - Granule Position. Always 0 for header
    ZeroMemory(&(_btOggHeader[0x16]), 0x04); // 0x16 - 0x19 - CRC. We'll calculate the CRC in the packagin function
    _btOggHeader[0x1A] = 0x01; // Always 1 segment

    // Segment Table
    _dwSegmentTableSize = 0x01; // just 1 byte to represent 0x1E
    _btVorbisSegmentTable[0x00] = 0x1E; // The first segment table is always 0x1E bytes

    // Vorbis Info Block (Audio info like Audio sample/bitrate/channel, Encoder and Codebook)
    CopyMemory(_btVorbisBlockData, &(pHeaderAndCodebook[btTotalBlocks + 0x01]), btInfoBlockSize); // 'btInfoBlockSize' can be used interchangably with 0x1E
    _dwTotalVorbisBlockSize = 0x1E; // Info block is always 0x1E bytes anyways
    this->_EncapsulateVorbisToOgg(0x02); // Page type. 2: b_o_s
    // -------------------------------------------------------------------------


    // Second Ogg Page ---------------------------------------------------------
    // HEADER ------------------------------------------------------------------
    _u64GranulePos = 0; // 0x06 - 0x0D - Granule Position. Always 0 for comment/codebook block
    CopyMemory(&(_btOggHeader[0x12]), &_dwPageSequence, 0x04); // 0x12 - 0x15 - Page Sequence. 1 for this block.
    ZeroMemory(&(_btOggHeader[0x16]), 0x04); // 0x16 - 0x19 - CRC. We'll calculate the CRC in the packagin function

    // Segment Table
    // Comment block split in blocks of 0xFF
    dwGen = btCommentBlockSize;
    dwIndex = 0;
    do
    {
        if (dwGen > 0xFF)
        {
            _btVorbisSegmentTable[dwIndex] = 0xFF;
            dwGen -= 0xFF;
        }
        else
        {
            _btVorbisSegmentTable[dwIndex] = (BYTE)dwGen;
            dwGen = 0;
        }
        dwIndex++;
    } while (dwGen > 0);

    // Codebook block split in blocks of 0xFF
    dwGen = dwCodebookBlockSize;
    do
    {
        if (dwGen > 0xFF)
        {
            _btVorbisSegmentTable[dwIndex] = 0xFF;
            dwGen -= 0xFF;
        }
        else
        {
            _btVorbisSegmentTable[dwIndex] = (BYTE)dwGen;
            dwGen = 0;
        }
        dwIndex++;
    } while (dwGen > 0);
    _dwSegmentTableSize = dwIndex;
    _btOggHeader[0x1A] = (BYTE)_dwSegmentTableSize;

    // Actual Data
    CopyMemory(&(_btVorbisBlockData[0x00]), &(pHeaderAndCodebook[btTotalBlocks + 0x01 + btInfoBlockSize]), btCommentBlockSize); // Copy the core comment block data
    CopyMemory(&(_btVorbisBlockData[btCommentBlockSize]), &(pHeaderAndCodebook[btTotalBlocks + 0x01 + btInfoBlockSize + btCommentBlockSize]), dwCodebookBlockSize); // Copy the core codebook block data
    _dwTotalVorbisBlockSize = btCommentBlockSize + dwCodebookBlockSize;
    this->_EncapsulateVorbisToOgg(0x00); // Page type. 0: Cotnuation page
    // -------------------------------------------------------------------------

    // All set. Input has been validated and we are set to start packaging further vorbis audio blocks
    _bInputValidated = TRUE;
    return;
}

// Keep adding actual vorbis audio data and package them into ogg pages
// once the segment table length is hit
BOOL ISOMTOOGGVORBIS::AddVorbisBlock(const LPBYTE pVorbisAudioBlock, DWORD dataBlockSize, unsigned long long u64GranulePos, BOOL lastBlock)
{
    DWORD       dwDataBlockSize;
    BYTE        btCurrSegmentSpan = 0x00;

    if (!_bInputValidated) return FALSE;
    if (u64GranulePos == 0x00) return FALSE;

    // Pick the granule pos. We will use the CURRENT granule pos to update in the PREVIOUS page, if limit is hit
    _u64GranulePos = u64GranulePos; // Note that granule pos is already cumulative sample count. So, no need to keep adding
    btCurrSegmentSpan = (BYTE)((int)dataBlockSize / 0xFF); // int division
    btCurrSegmentSpan += (dataBlockSize % 0xFF == 0 ? 0 : 1); // need 1 more byte if there are remainders

    // If the segment table limit is hit, pack the data (so far) into an ogg page
    if ((btCurrSegmentSpan + _dwSegmentTableSize) > _MAX_SEGMENT_TABLE_SIZE)
    {
        // Write from segment/body streams into the main audio steam and reset dwDataBlockSize to 0;
        this->_EncapsulateVorbisToOgg();
    }

    // Start pushing/preparing the data for the next ogg page
    // Do not worry about the markers. They are set/reset in the _EncapsulateVorbisToOgg() method as needed
    dwDataBlockSize = dataBlockSize;
    do
    {
        if (dwDataBlockSize > 0xFF)
        {
            _btVorbisSegmentTable[_dwSegmentTableSize++] = 0xFF;
            dwDataBlockSize -= 0xFF;
        }
        else
        {
            _btVorbisSegmentTable[_dwSegmentTableSize++] = (BYTE)dwDataBlockSize;
            dwDataBlockSize = 0x00;
        }
    } while (dwDataBlockSize);
    CopyMemory(&(_btVorbisBlockData[_dwTotalVorbisBlockSize]), pVorbisAudioBlock, dataBlockSize);
    _dwTotalVorbisBlockSize += dataBlockSize; // Keep tracking the written bytes that are piling up.

    // If this is the last block, then wrap up everything.
    if (lastBlock)
        this->_EncapsulateVorbisToOgg(0x04); // Write from segment/body streams into the main audio steam and reset dwDataBlockSize to 0;
    return TRUE;
}

// Pluck the whole data from the stream
BOOL ISOMTOOGGVORBIS::GetFullAudioTrack(LPBYTE *ppTrackData, LPDWORD pTrackLength)
{
    ULARGE_INTEGER      uliTrackSize { 0 };
    LARGE_INTEGER       liStart { 0 };
    DWORD               dwGen = 0;

    if (!ppTrackData || !pTrackLength) return FALSE;
    if (!this->_bInputValidated) return FALSE;
    IStream_Size(_pFullAudioTrack, &uliTrackSize);
    *pTrackLength = uliTrackSize.LowPart;
    *ppTrackData = NULL;
    if (*pTrackLength > 0)
    {
        *ppTrackData = (LPBYTE)LocalAlloc(LPTR, *pTrackLength);
        // Do NOT forget to rewind the stream to the start!
        _pFullAudioTrack->Seek(liStart, STREAM_SEEK_SET, NULL);
        _pFullAudioTrack->Read(*ppTrackData, *pTrackLength, &dwGen);
    }
    return TRUE;

}

// Done
ISOMTOOGGVORBIS::~ISOMTOOGGVORBIS()
{
    SafeRelease(&_pFullAudioTrack);
    return;
}
