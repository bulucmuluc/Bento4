/*****************************************************************
|
|    AP4 - Sample Objects
|
|    Copyright 2002-2005 Gilles Boccon-Gibod & Julien Boeuf
|
|
|    This file is part of Bento4/AP4 (MP4 Atom Processing Library).
|
|    Unless you have obtained Bento4 under a difference license,
|    this version of Bento4 is Bento4|GPL.
|    Bento4|GPL is free software; you can redistribute it and/or modify
|    it under the terms of the GNU General Public License as published by
|    the Free Software Foundation; either version 2, or (at your option)
|    any later version.
|
|    Bento4|GPL is distributed in the hope that it will be useful,
|    but WITHOUT ANY WARRANTY; without even the implied warranty of
|    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|    GNU General Public License for more details.
|
|    You should have received a copy of the GNU General Public License
|    along with Bento4|GPL; see the file COPYING.  If not, write to the
|    Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
|    02111-1307, USA.
|
 ****************************************************************/

#ifndef _AP4_SAMPLE_H_
#define _AP4_SAMPLE_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"

/*----------------------------------------------------------------------
|   class references
+---------------------------------------------------------------------*/
class AP4_ByteStream;
class AP4_DataBuffer;

/*----------------------------------------------------------------------
|   AP4_Sample DO NOT DERIVE FROM THIS CLASS
+---------------------------------------------------------------------*/
class AP4_Sample 
{
public:
    // constructors and destructor
    AP4_Sample();
    AP4_Sample(const AP4_Sample& other);
    AP4_Sample(AP4_ByteStream& data_stream,
               AP4_Position    offset,
               AP4_Size        size,
               AP4_Ordinal     description_index,
               AP4_TimeStamp   dts,
               AP4_UI32        cts_offset = 0,
               bool            sync_flag = true);
    ~AP4_Sample(); // not virtual on purpose: do not derive from it

    // operators
    AP4_Sample&     operator=(const AP4_Sample& other);

    // methods
    AP4_Result      ReadData(AP4_DataBuffer& data);
    AP4_Result      ReadData(AP4_DataBuffer& data, 
                             AP4_Size        size, 
                             AP4_Size        offset = 0);

    // sample properties accessors
    AP4_ByteStream* GetDataStream();
    void            SetDataStream(AP4_ByteStream& stream);
    AP4_Position    GetOffset() const { return m_Offset; }
    void            SetOffset(AP4_Position offset) { m_Offset = offset; }
    AP4_Size        GetSize() { return m_Size; }
    void            SetSize(AP4_Size size) { m_Size = size; }
    AP4_Ordinal     GetDescriptionIndex() const { return m_DescriptionIndex; }
    void            SetDescriptionIndex(AP4_Ordinal index) { m_DescriptionIndex = index; }
    AP4_TimeStamp   GetDts() const { return m_Dts; }
    void            SetDts(AP4_TimeStamp dts) { m_Dts = dts; }
    AP4_TimeStamp   GetCts() const { return m_Cts; }
    void            SetCts(AP4_TimeStamp cts) { m_Cts = cts; }
    bool            IsSync() const { return m_IsSync; }
    void            SetSync(bool is_sync) { m_IsSync = is_sync; }

protected:
    AP4_ByteStream* m_DataStream;
    AP4_Position    m_Offset;
    AP4_Size        m_Size;
    AP4_Ordinal     m_DescriptionIndex;
    AP4_TimeStamp   m_Dts;
    AP4_TimeStamp   m_Cts;
    bool            m_IsSync;
};

#endif // _AP4_SAMPLE_H_