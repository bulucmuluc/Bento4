/*****************************************************************
|
|    AP4 - MP4 File Info
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

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>

#include "Ap4.h"

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
#define BANNER "MP4 File Info - Version 1.0\n"\
               "(Bento4 Version " AP4_VERSION_STRING ")\n"\
               "(c) 2002-2006 Gilles Boccon-Gibod & Julien Boeuf"
 
/*----------------------------------------------------------------------
|   PrintUsageAndExit
+---------------------------------------------------------------------*/
static void
PrintUsageAndExit()
{
    fprintf(stderr, 
            BANNER 
            "\n\nusage: mp4info [options] <input>\n"
            "Options:\n"
            "  --verbose: show sample details\n");
    exit(1);
}

/*----------------------------------------------------------------------
|   ShowPayload
+---------------------------------------------------------------------*/
static void
ShowPayload(AP4_Atom* atom, bool ascii = false)
{
    AP4_UI64 payload_size = atom->GetSize()-8;
    if (payload_size <= 1024) {
        AP4_MemoryByteStream* payload = new AP4_MemoryByteStream();
        atom->Write(*payload);
        if (ascii) {
            // ascii
            payload->WriteUI08(0); // terminate with a NULL character
            AP4_Debug("%s", (char*)payload->GetData()+atom->GetHeaderSize());
        } else {
            // hex
            for (unsigned int i=0; i<payload_size; i++) {
                AP4_Debug("%02x", (unsigned char)payload->GetData()[atom->GetHeaderSize()+i]);
            }
        }
        payload->Release();
    }
}

/*----------------------------------------------------------------------
|   ShowProtectedSampleDescription
+---------------------------------------------------------------------*/
static void
ShowProtectedSampleDescription(AP4_ProtectedSampleDescription* desc)
{
    if (desc == NULL) return;
    AP4_Debug("    [ENCRYPTED]\n");
    char coding[5];
    AP4_FormatFourChars(coding, desc->GetFormat());
    AP4_Debug("      Coding:         %s\n", coding);
    AP4_UI32 st = desc->GetSchemeType();
    AP4_Debug("      Scheme Type:    %c%c%c%c\n", 
        (char)((st>>24) & 0xFF),
        (char)((st>>16) & 0xFF),
        (char)((st>> 8) & 0xFF),
        (char)((st    ) & 0xFF));
    AP4_Debug("      Scheme Version: %d\n", desc->GetSchemeVersion());
    AP4_Debug("      Scheme URI:     %s\n", desc->GetSchemeUri().GetChars());
    AP4_ProtectionSchemeInfo* scheme_info = desc->GetSchemeInfo();
    if (scheme_info == NULL) return;
    AP4_ContainerAtom* schi = scheme_info->GetSchiAtom();
    if (schi == NULL) return;
    if (desc->GetSchemeType() == AP4_PROTECTION_SCHEME_TYPE_IAEC) {
        AP4_Debug("      iAEC Scheme Info:\n");
        AP4_IkmsAtom* ikms = dynamic_cast<AP4_IkmsAtom*>(schi->FindChild("iKMS"));
        if (ikms) {
            AP4_Debug("        KMS URI:              %s\n", ikms->GetKmsUri().GetChars());
        }
        AP4_IsfmAtom* isfm = dynamic_cast<AP4_IsfmAtom*>(schi->FindChild("iSFM"));
        if (isfm) {
            AP4_Debug("        Selective Encryption: %s\n", isfm->GetSelectiveEncryption()?"yes":"no");
            AP4_Debug("        Key Indicator Length: %d\n", isfm->GetKeyIndicatorLength());
            AP4_Debug("        IV Length:            %d\n", isfm->GetIvLength());
        }
        AP4_IsltAtom* islt = dynamic_cast<AP4_IsltAtom*>(schi->FindChild("iSLT"));
        if (islt) {
            AP4_Debug("        Salt:                 ");
            for (unsigned int i=0; i<8; i++) {
                AP4_Debug("%02x",islt->GetSalt()[i]);
            }
            AP4_Debug("\n");
        }
    } else if (desc->GetSchemeType() == AP4_PROTECTION_SCHEME_TYPE_OMA) {
        AP4_Debug("      odkm Scheme Info:\n");
        AP4_OdafAtom* odaf = dynamic_cast<AP4_OdafAtom*>(schi->FindChild("odkm/odaf"));
        if (odaf) {
            AP4_Debug("        Selective Encryption: %s\n", odaf->GetSelectiveEncryption()?"yes":"no");
            AP4_Debug("        Key Indicator Length: %d\n", odaf->GetKeyIndicatorLength());
            AP4_Debug("        IV Length:            %d\n", odaf->GetIvLength());
        }
        AP4_OhdrAtom* ohdr = dynamic_cast<AP4_OhdrAtom*>(schi->FindChild("odkm/ohdr"));
        if (ohdr) {
            const char* encryption_method = "";
            switch (ohdr->GetEncryptionMethod()) {
                case AP4_OMA_DCF_ENCRYPTION_METHOD_NULL:    encryption_method = "NULL";    break;
                case AP4_OMA_DCF_ENCRYPTION_METHOD_AES_CTR: encryption_method = "AES-CTR"; break;
                case AP4_OMA_DCF_ENCRYPTION_METHOD_AES_CBC: encryption_method = "AES-CBC"; break;
                default:                                    encryption_method = "UNKNOWN"; break;
            }
            AP4_Debug("        Encryption Method: %s\n", encryption_method);
            AP4_Debug("        Content ID:        %s\n", ohdr->GetContentId().GetChars());
            AP4_Debug("        Rights Issuer URL: %s\n", ohdr->GetRightsIssuerUrl().GetChars());
        }
    } else if (desc->GetSchemeType() == AP4_PROTECTION_SCHEME_TYPE_ITUNES) {
        AP4_Debug("      itun Scheme Info:\n");
        AP4_Atom* name = schi->FindChild("name");
        if (name) {
            AP4_Debug("        Name:    ");
            ShowPayload(name, true);
            AP4_Debug("\n");
        }
        AP4_Atom* user = schi->FindChild("user");
        if (user) {
            AP4_Debug("        User ID: ");
            ShowPayload(user);
            AP4_Debug("\n");
        }
        AP4_Atom* key = schi->FindChild("key ");
        if (key) {
            AP4_Debug("        Key ID:  ");
            ShowPayload(key);
            AP4_Debug("\n");
        }
        AP4_Atom* iviv = schi->FindChild("iviv");
        if (iviv) {
            AP4_Debug("        IV:      ");
            ShowPayload(iviv);
            AP4_Debug("\n");
        }
    }
}

/*----------------------------------------------------------------------
|   ShowMpegAudioSampleDescription
+---------------------------------------------------------------------*/
static void
ShowMpegAudioSampleDescription(AP4_MpegAudioSampleDescription* mpeg_audio_desc)
{
    AP4_MpegAudioSampleDescription::Mpeg4AudioObjectType object_type = 
        mpeg_audio_desc->GetMpeg4AudioObjectType();
    const char* object_type_string = AP4_MpegAudioSampleDescription::GetMpeg4AudioObjectTypeString(object_type);
    AP4_Debug("    MPEG-4 Audio Object Type: %s\n", object_type_string);
}

/*----------------------------------------------------------------------
|   ShowSampleDescription
+---------------------------------------------------------------------*/
static void
ShowSampleDescription(AP4_SampleDescription* desc)
{
    if (desc->GetType() == AP4_SampleDescription::TYPE_PROTECTED) {
        AP4_ProtectedSampleDescription* prot_desc = dynamic_cast<AP4_ProtectedSampleDescription*>(desc);
        ShowProtectedSampleDescription(prot_desc);
        desc = prot_desc->GetOriginalSampleDescription();
    }
    char coding[5];
    AP4_FormatFourChars(coding, desc->GetFormat());
    AP4_Debug(    "    Coding:      %s\n", coding);
    if (desc->GetType() == AP4_SampleDescription::TYPE_MPEG) {
        // MPEG sample description
        AP4_MpegSampleDescription* mpeg_desc = dynamic_cast<AP4_MpegSampleDescription*>(desc);

        AP4_Debug("    Stream Type: %s\n", mpeg_desc->GetStreamTypeString(mpeg_desc->GetStreamType()));
        AP4_Debug("    Object Type: %s\n", mpeg_desc->GetObjectTypeString(mpeg_desc->GetObjectTypeId()));
        AP4_Debug("    Max Bitrate: %d\n", mpeg_desc->GetMaxBitrate());
        AP4_Debug("    Avg Bitrate: %d\n", mpeg_desc->GetAvgBitrate());
        AP4_Debug("    Buffer Size: %d\n", mpeg_desc->GetBufferSize());
        
        if (mpeg_desc->GetObjectTypeId() == AP4_OTI_MPEG4_AUDIO) {
            AP4_MpegAudioSampleDescription* mpeg_audio_desc = dynamic_cast<AP4_MpegAudioSampleDescription*>(mpeg_desc);
            if (mpeg_audio_desc) ShowMpegAudioSampleDescription(mpeg_audio_desc);
        }
    }
    AP4_AudioSampleDescription* audio_desc = 
        dynamic_cast<AP4_AudioSampleDescription*>(desc);
    if (audio_desc) {
        // Audio sample description
        AP4_Debug("    Sample Rate: %d\n", audio_desc->GetSampleRate());
        AP4_Debug("    Sample Size: %d\n", audio_desc->GetSampleSize());
        AP4_Debug("    Channels:    %d\n", audio_desc->GetChannelCount());
    }
    AP4_VideoSampleDescription* video_desc = 
        dynamic_cast<AP4_VideoSampleDescription*>(desc);
    if (video_desc) {
        // Video sample description
        AP4_Debug("    Width:       %d\n", video_desc->GetWidth());
        AP4_Debug("    Height:      %d\n", video_desc->GetHeight());
        AP4_Debug("    Depth:       %d\n", video_desc->GetDepth());
    }
    if (desc->GetFormat() == AP4_ATOM_TYPE_AVC1) {
        // AVC sample description
        AP4_AvccAtom* avcc = (AP4_AvccAtom*)desc->GetDetails().GetChild(AP4_ATOM_TYPE_AVCC);
        if (avcc) {
            const char* profile_name = AP4_AvccAtom::GetProfileName(avcc->GetProfile());
            if (profile_name) {
                AP4_Debug("    AVC Profile:          %s\n", profile_name);
            } else {
                AP4_Debug("    AVC Profile:          %d\n", avcc->GetProfile());
            }
            AP4_Debug("    AVC Profile Compat:   %x\n", avcc->GetProfileCompatibility());
            AP4_Debug("    AVC Level:            %d\n", avcc->GetLevel());
            AP4_Debug("    AVC NALU Length Size: %d\n", avcc->GetNaluLengthSize());
        }
    }
}

/*----------------------------------------------------------------------
|   ShowTrackInfo
+---------------------------------------------------------------------*/
static void
ShowTrackInfo(AP4_Track* track, bool verbose = false)
{
	AP4_Debug("  id:           %ld\n", track->GetId());
    AP4_Debug("  type:         ");
    switch (track->GetType()) {
        case AP4_Track::TYPE_AUDIO:   AP4_Debug("Audio\n"); break;
        case AP4_Track::TYPE_VIDEO:   AP4_Debug("Video\n"); break;
        case AP4_Track::TYPE_HINT:    AP4_Debug("Hint\n");  break;
        case AP4_Track::TYPE_SYSTEM:  AP4_Debug("System\n");  break;
        case AP4_Track::TYPE_TEXT:    AP4_Debug("Text\n");  break;
        case AP4_Track::TYPE_JPEG:    AP4_Debug("Jpeg\n");  break;
        default: {
            char hdlr[5];
            AP4_FormatFourChars(hdlr, track->GetHandlerType());
            AP4_Debug("Unknown [");
            AP4_Debug(hdlr);
            AP4_Debug("]\n");
            break;
        }
    }
    AP4_Debug("  duration:     %ld ms\n", track->GetDurationMs());
    AP4_Debug("  timescale:    %ld\n", track->GetMediaTimeScale());
    AP4_Debug("  sample count: %ld\n", track->GetSampleCount());
	AP4_Sample  sample;
	AP4_Ordinal index = 0;
    AP4_Ordinal desc_index = 0xFFFFFFFF;
    while (AP4_SUCCEEDED(track->GetSample(index, sample))) {
        if (sample.GetDescriptionIndex() != desc_index) {
            desc_index = sample.GetDescriptionIndex();
            AP4_Debug("  [%d]: Sample Description %d\n", index, desc_index);
            
            // get the sample description
            AP4_SampleDescription* sample_desc = 
                track->GetSampleDescription(desc_index);
            if (sample_desc != NULL) {
                ShowSampleDescription(sample_desc);
            }
        }
        if (verbose) {
            printf("[%08d] size=%6d dts=%8d, cts=%8d", 
                   index+1,
                   (int)sample.GetSize(),
                   (int)sample.GetDts(), 
                   (int)sample.GetCts());
            if (sample.IsSync()) {
                printf(" [Sync]\n");
            } else {
                printf("\n");
            }
        }
        index++;
    }
}

/*----------------------------------------------------------------------
|   ShowMovieInfo
+---------------------------------------------------------------------*/
static void
ShowMovieInfo(AP4_Movie* movie)
{
    AP4_Debug("Movie:\n");
    AP4_Debug("  duration:   %ld ms\n", movie->GetDurationMs());
    AP4_Debug("  time scale: %ld\n", movie->GetTimeScale());
    AP4_Debug("\n");
}

/*----------------------------------------------------------------------
|   ShowFileInfo
+---------------------------------------------------------------------*/
static void
ShowFileInfo(AP4_File* file)
{
    AP4_FtypAtom* file_type = file->GetFileType();
    if (file_type == NULL) return;
    char four_cc[5];

    AP4_FormatFourChars(four_cc, file_type->GetMajorBrand());
    AP4_Debug("File:\n");
    AP4_Debug("  major brand:      %s\n", four_cc);
    AP4_Debug("  minor version:    %x\n", file_type->GetMinorVersion());

    // compatible brands
    for (unsigned int i=0; i<file_type->GetCompatibleBrands().ItemCount(); i++) {
        AP4_UI32 cb = file_type->GetCompatibleBrands()[i];
        if (cb == 0) continue;
        AP4_FormatFourChars(four_cc, cb);
        AP4_Debug("  compatible brand: %s\n", four_cc);
    }
    AP4_Debug("\n");
}


/*----------------------------------------------------------------------
|   main
+---------------------------------------------------------------------*/
int
main(int argc, char** argv)
{
    if (argc < 2) {
        PrintUsageAndExit();
    }
    const char* filename = NULL;
    bool verbose = false;

    while (char* arg = *++argv) {
        if (!strcmp(arg, "--verbose")) {
            verbose = true;
        } else {
            if (filename == NULL) {
                filename = arg;
            } else {
                fprintf(stderr, "ERROR: unexpected argument '%s'\n", arg);
                return 1;
            }
        }   
    }
    if (filename == NULL) {
        fprintf(stderr, "ERROR: filename missing\n");
        return 1;
    }

    AP4_ByteStream* input;
    try {
        input = new AP4_FileByteStream(filename,
                               AP4_FileByteStream::STREAM_MODE_READ);
    } catch (AP4_Exception) {
        fprintf(stderr, "ERROR: cannot open input file (%s)\n", argv[1]);
        return 1;
    }

    AP4_File* file = new AP4_File(*input);
    ShowFileInfo(file);

    AP4_Movie* movie = file->GetMovie();
    if (movie != NULL) {
        ShowMovieInfo(movie);

        AP4_List<AP4_Track>& tracks = movie->GetTracks();
        AP4_Debug("Found %d Tracks\n", tracks.ItemCount());

        AP4_List<AP4_Track>::Item* track_item = tracks.FirstItem();
        int index = 1;
        while (track_item) {
            AP4_Debug("Track %d:\n", index); 
            index++;
            ShowTrackInfo(track_item->GetData(), verbose);
            track_item = track_item->GetNext();
        }
    } else {
        AP4_Debug("No movie found in the file\n");
    }

    delete file;
    input->Release();

    return 0;
}