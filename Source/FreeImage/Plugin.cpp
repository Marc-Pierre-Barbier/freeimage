// =====================================================================
// FreeImage Plugin Interface
//
// Design and implementation by
// - Floris van den Berg (floris@geekhq.nl)
// - Rui Lopes (ruiglopes@yahoo.com)
// - Detlev Vendt (detlev.vendt@brillit.de)
// - Petr Pytelka (pyta@lightcomp.com)
//
// This file is part of FreeImage 3
//
// COVERED CODE IS PROVIDED UNDER THIS LICENSE ON AN "AS IS" BASIS, WITHOUT WARRANTY
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, WITHOUT LIMITATION, WARRANTIES
// THAT THE COVERED CODE IS FREE OF DEFECTS, MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE
// OR NON-INFRINGING. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE COVERED
// CODE IS WITH YOU. SHOULD ANY COVERED CODE PROVE DEFECTIVE IN ANY RESPECT, YOU (NOT
// THE INITIAL DEVELOPER OR ANY OTHER CONTRIBUTOR) ASSUME THE COST OF ANY NECESSARY
// SERVICING, REPAIR OR CORRECTION. THIS DISCLAIMER OF WARRANTY CONSTITUTES AN ESSENTIAL
// PART OF THIS LICENSE. NO USE OF ANY COVERED CODE IS AUTHORIZED HEREUNDER EXCEPT UNDER
// THIS DISCLAIMER.
//
// Use at your own risk!
// =====================================================================

#ifdef _MSC_VER
#pragma warning (disable : 4786) // identifier was truncated to 'number' characters
#endif

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <ctype.h>
#endif // _WIN32

#include "FreeImage.h"
#include "Utilities.h"
#include "FreeImageIO.h"
#include "Plugin.h"

#include "../Metadata/FreeImageTag.h"

// =====================================================================

using namespace std;

// =====================================================================
// Plugin search list
// =====================================================================

const char *
s_search_list[] = {
	"",
	"plugins\\",
};

static int s_search_list_size = sizeof(s_search_list) / sizeof(char *);
static PluginList *s_plugins = NULL;
static int s_plugin_reference_count = 0;


// =====================================================================
// Reimplementation of stricmp (it is not supported on some systems)
// =====================================================================

int
FreeImage_stricmp(const char *s1, const char *s2) {
	int c1, c2;

	do {
		c1 = tolower(*s1++);
		c2 = tolower(*s2++);
	} while (c1 && c1 == c2);

	return c1 - c2;
}

// =====================================================================
//  Implementation of PluginList
// =====================================================================

PluginList::PluginList() :
m_plugin_map(),
m_node_count(0) {
}

FREE_IMAGE_FORMAT
PluginList::AddNode(FI_InitProc init_proc, FREE_IMAGE_FORMAT fif, void *instance, const char *format, const char *description, const char *extension, const char *regexpr) {
	if (init_proc != NULL) {
		PluginNode *node = new(std::nothrow) PluginNode;
		Plugin *plugin = new(std::nothrow) Plugin;
		if(!node || !plugin) {
			if(node) delete node;
			if(plugin) delete plugin;
			FreeImage_OutputMessageProc(FIF_UNKNOWN, FI_MSG_ERROR_MEMORY);
			return FIF_UNKNOWN;
		}

		memset(plugin, 0, sizeof(Plugin));

		// fill-in the plugin structure
		// note we have memset to 0, so all unset pointers should be NULL)

		init_proc(plugin, (int)fif);

		// get the format string (two possible ways)

		const char *the_format = NULL;

		if (format != NULL) {
			the_format = format;
		} else if (plugin->format_proc != NULL) {
			the_format = plugin->format_proc();
		}

		// add the node if it wasn't there already

		if (the_format != NULL) {
			node->m_id = (int)fif;
			node->m_instance = instance;
			node->m_plugin = plugin;
			node->m_format = format;
			node->m_description = description;
			node->m_extension = extension;
			node->m_regexpr = regexpr;
			node->m_enabled = TRUE;

			m_plugin_map[(int)fif] = node;

			return (FREE_IMAGE_FORMAT)node->m_id;
		}

		// something went wrong while allocating the plugin... cleanup

		delete plugin;
		delete node;
	}

	return FIF_UNKNOWN;
}

PluginNode *
PluginList::FindNodeFromFormat(const char *format) {
	for (map<int, PluginNode *>::iterator i = m_plugin_map.begin(); i != m_plugin_map.end(); ++i) {
		const char *the_format = ((*i).second->m_format != NULL) ? (*i).second->m_format : (*i).second->m_plugin->format_proc();

		if ((*i).second->m_enabled) {
			if (FreeImage_stricmp(the_format, format) == 0) {
				return (*i).second;
			}
		}
	}

	return NULL;
}

PluginNode *
PluginList::FindNodeFromMime(const char *mime) {
	for (map<int, PluginNode *>::iterator i = m_plugin_map.begin(); i != m_plugin_map.end(); ++i) {
		const char *the_mime = ((*i).second->m_plugin->mime_proc != NULL) ? (*i).second->m_plugin->mime_proc() : "";

		if ((*i).second->m_enabled) {
			if ((the_mime != NULL) && (strcmp(the_mime, mime) == 0)) {
				return (*i).second;
			}
		}
	}

	return NULL;
}

PluginNode *
PluginList::FindNodeFromFIF(int node_id) {
	map<int, PluginNode *>::iterator i = m_plugin_map.find(node_id);

	if (i != m_plugin_map.end()) {
		return (*i).second;
	}

	return NULL;
}

int
PluginList::Size() const {
	return (int)m_plugin_map.size();
}

bool
PluginList::IsEmpty() const {
	return m_plugin_map.empty();
}

PluginList::~PluginList() {
	for (map<int, PluginNode *>::iterator i = m_plugin_map.begin(); i != m_plugin_map.end(); ++i) {
#ifdef _WIN32
		if ((*i).second->m_instance != NULL) {
			FreeLibrary((HINSTANCE)(*i).second->m_instance);
		}
#endif
		delete (*i).second->m_plugin;
		delete ((*i).second);
	}
}

// =====================================================================
// Retrieve a pointer to the plugin list container
// =====================================================================

PluginList * DLL_CALLCONV
FreeImage_GetPluginList() {
	return s_plugins;
}

// =====================================================================
// Plugin System Initialization
// =====================================================================

void DLL_CALLCONV
FreeImage_Initialise(bool load_local_plugins_only) {
	if (s_plugin_reference_count++ == 0) {

		/*
		Note: initialize all singletons here
		in order to avoid race conditions with multi-threading
		*/

		// initialise the TagLib singleton
		TagLib& s = TagLib::instance();

		// internal plugin initialization

		s_plugins = new(std::nothrow) PluginList;

		if (s_plugins) {
			/* NOTE :
			The order used to initialize internal plugins below MUST BE the same order
			as the one used to define the FREE_IMAGE_FORMAT enum.
			*/
			#ifdef USE_JPEG
			s_plugins->AddNode(InitJPEG, FIF_JPEG);
			#endif
			#ifdef USE_PNG
			s_plugins->AddNode(InitPNG, FIF_PNG);
			#endif
			#ifdef USE_TIFF
			s_plugins->AddNode(InitTIFF, FIF_TIFF);
			s_plugins->AddNode(InitG3, FIF_FAXG3);
			#endif
			#ifdef USE_OpenEXR
			s_plugins->AddNode(InitEXR, FIF_EXR);
			#endif
			#ifdef USE_OpenJPEG
			s_plugins->AddNode(InitJ2K, FIF_J2K);
			s_plugins->AddNode(InitJP2, FIF_JP2);
			#endif
			#ifdef USE_RawLite
			s_plugins->AddNode(InitRAW, FIF_RAW);
			#endif
			#ifdef USE_WEBP
			s_plugins->AddNode(InitWEBP, FIF_WEBP);
			#endif
			#ifdef USE_JXR
			s_plugins->AddNode(InitJXR, FIF_JXR);
			#endif
			s_plugins->AddNode(InitPFM, FIF_PFM);
			s_plugins->AddNode(InitPICT, FIF_PICT);
			s_plugins->AddNode(InitWBMP, FIF_WBMP);
			s_plugins->AddNode(InitPSD, FIF_PSD);
			s_plugins->AddNode(InitCUT, FIF_CUT);
			s_plugins->AddNode(InitXBM, FIF_XBM);
			s_plugins->AddNode(InitXPM, FIF_XPM);
			s_plugins->AddNode(InitDDS, FIF_DDS);
	        s_plugins->AddNode(InitGIF, FIF_GIF);
	        s_plugins->AddNode(InitHDR, FIF_HDR);
			s_plugins->AddNode(InitSGI, FIF_SGI);
			s_plugins->AddNode(InitBMP, FIF_BMP);
			s_plugins->AddNode(InitICO, FIF_ICO);
			s_plugins->AddNode(InitJNG, FIF_JNG);
			s_plugins->AddNode(InitKOALA, FIF_KOALA);
			s_plugins->AddNode(InitIFF, FIF_IFF);
			s_plugins->AddNode(InitMNG, FIF_MNG);
			s_plugins->AddNode(InitPCD, FIF_PCD);
			s_plugins->AddNode(InitPCX, FIF_PCX);
			s_plugins->AddNode(InitRAS, FIF_RAS);
			s_plugins->AddNode(InitTARGA, FIF_TARGA);
			s_plugins->AddNode(InitPNM, FIF_PBM, NULL, "PBM", "Portable Bitmap (ASCII)", "pbm", "^P1");
			s_plugins->AddNode(InitPNM, FIF_PBMRAW, NULL, "PBMRAW", "Portable Bitmap (RAW)", "pbm", "^P4");
			s_plugins->AddNode(InitPNM, FIF_PGM, NULL, "PGM", "Portable Greymap (ASCII)", "pgm", "^P2");
			s_plugins->AddNode(InitPNM, FIF_PGMRAW, NULL, "PGMRAW", "Portable Greymap (RAW)", "pgm", "^P5");
			s_plugins->AddNode(InitPNM, FIF_PPM, NULL, "PPM", "Portable Pixelmap (ASCII)", "ppm", "^P3");
			s_plugins->AddNode(InitPNM, FIF_PPMRAW, NULL, "PPMRAW", "Portable Pixelmap (RAW)", "ppm", "^P6");
		}
	}
}

void DLL_CALLCONV
FreeImage_DeInitialise() {
	--s_plugin_reference_count;

	if (s_plugin_reference_count == 0) {
		delete s_plugins;
	}
}

// =====================================================================
// Open and close a bitmap
// =====================================================================

void * DLL_CALLCONV
FreeImage_Open(PluginNode *node, FreeImageIO *io, fi_handle handle, bool open_for_reading) {
	if (node->m_plugin->open_proc != NULL) {
       return node->m_plugin->open_proc(io, handle, open_for_reading);
	}

	return NULL;
}

void DLL_CALLCONV
FreeImage_Close(PluginNode *node, FreeImageIO *io, fi_handle handle, void *data) {
	if (node->m_plugin->close_proc != NULL) {
		node->m_plugin->close_proc(io, handle, data);
	}
}

// =====================================================================
// Plugin System Load/Save Functions
// =====================================================================

FIBITMAP * DLL_CALLCONV
FreeImage_LoadFromHandle(FREE_IMAGE_FORMAT fif, FreeImageIO *io, fi_handle handle, int flags) {
	if ((fif >= 0) && (fif < FreeImage_GetFIFCount())) {
		PluginNode *node = s_plugins->FindNodeFromFIF(fif);

		if (node != NULL) {
			if(node->m_plugin->load_proc != NULL) {
				void *data = FreeImage_Open(node, io, handle, TRUE);

				FIBITMAP *bitmap = node->m_plugin->load_proc(io, handle, -1, flags, data);

				FreeImage_Close(node, io, handle, data);

				return bitmap;
			}
		}
	}

	return NULL;
}

FIBITMAP * DLL_CALLCONV
FreeImage_Load(FREE_IMAGE_FORMAT fif, const char *filename, int flags) {
	FreeImageIO io;
	SetDefaultIO(&io);
	FILE *handle = fopen(filename, "rb");

	if (handle) {
		FIBITMAP *bitmap = FreeImage_LoadFromHandle(fif, &io, (fi_handle)handle, flags);

		fclose(handle);

		return bitmap;
	} else {
		FreeImage_OutputMessageProc((int)fif, "FreeImage_Load: failed to open file %s", filename);
	}

	return NULL;
}

FIBITMAP * DLL_CALLCONV
FreeImage_LoadU(FREE_IMAGE_FORMAT fif, const wchar_t *filename, int flags) {
	FreeImageIO io;
	SetDefaultIO(&io);
#ifdef _WIN32
	FILE *handle = _wfopen(filename, L"rb");

	if (handle) {
		FIBITMAP *bitmap = FreeImage_LoadFromHandle(fif, &io, (fi_handle)handle, flags);

		fclose(handle);

		return bitmap;
	} else {
		FreeImage_OutputMessageProc((int)fif, "FreeImage_LoadU: failed to open input file");
	}
#endif
	return NULL;
}

bool DLL_CALLCONV
FreeImage_SaveToHandle(FREE_IMAGE_FORMAT fif, FIBITMAP *dib, FreeImageIO *io, fi_handle handle, int flags) {
	// cannot save "header only" formats
	if(FreeImage_HasPixels(dib) == FALSE) {
		FreeImage_OutputMessageProc((int)fif, "FreeImage_SaveToHandle: cannot save \"header only\" formats");
		return FALSE;
	}

	if ((fif >= 0) && (fif < FreeImage_GetFIFCount())) {
		PluginNode *node = s_plugins->FindNodeFromFIF(fif);

		if (node) {
			if(node->m_plugin->save_proc != NULL) {
				void *data = FreeImage_Open(node, io, handle, FALSE);

				bool result = node->m_plugin->save_proc(io, dib, handle, -1, flags, data);

				FreeImage_Close(node, io, handle, data);

				return result;
			}
		}
	}

	return FALSE;
}


bool DLL_CALLCONV
FreeImage_Save(FREE_IMAGE_FORMAT fif, FIBITMAP *dib, const char *filename, int flags) {
	FreeImageIO io;
	SetDefaultIO(&io);

	FILE *handle = fopen(filename, "w+b");

	if (handle) {
		bool success = FreeImage_SaveToHandle(fif, dib, &io, (fi_handle)handle, flags);

		fclose(handle);

		return success;
	} else {
		FreeImage_OutputMessageProc((int)fif, "FreeImage_Save: failed to open file %s", filename);
	}

	return FALSE;
}

bool DLL_CALLCONV
FreeImage_SaveU(FREE_IMAGE_FORMAT fif, FIBITMAP *dib, const wchar_t *filename, int flags) {
	FreeImageIO io;
	SetDefaultIO(&io);
#ifdef _WIN32
	FILE *handle = _wfopen(filename, L"w+b");

	if (handle) {
		bool success = FreeImage_SaveToHandle(fif, dib, &io, (fi_handle)handle, flags);

		fclose(handle);

		return success;
	} else {
		FreeImage_OutputMessageProc((int)fif, "FreeImage_SaveU: failed to open output file");
	}
#endif
	return FALSE;
}

// =====================================================================
// Plugin construction + enable/disable functions
// =====================================================================

int DLL_CALLCONV
FreeImage_SetPluginEnabled(FREE_IMAGE_FORMAT fif, bool enable) {
	if (s_plugins != NULL) {
		PluginNode *node = s_plugins->FindNodeFromFIF(fif);

		if (node != NULL) {
			bool previous_state = node->m_enabled;

			node->m_enabled = enable;

			return previous_state;
		}
	}

	return -1;
}

int DLL_CALLCONV
FreeImage_IsPluginEnabled(FREE_IMAGE_FORMAT fif) {
	if (s_plugins != NULL) {
		PluginNode *node = s_plugins->FindNodeFromFIF(fif);

		return (node != NULL) ? node->m_enabled : FALSE;
	}

	return -1;
}

// =====================================================================
// Plugin Access Functions
// =====================================================================

int DLL_CALLCONV
FreeImage_GetFIFCount() {
	return (s_plugins != NULL) ? s_plugins->Size() : 0;
}

FREE_IMAGE_FORMAT DLL_CALLCONV
FreeImage_GetFIFFromFormat(const char *format) {
	if (s_plugins != NULL) {
		PluginNode *node = s_plugins->FindNodeFromFormat(format);

		return (node != NULL) ? (FREE_IMAGE_FORMAT)node->m_id : FIF_UNKNOWN;
	}

	return FIF_UNKNOWN;
}

FREE_IMAGE_FORMAT DLL_CALLCONV
FreeImage_GetFIFFromMime(const char *mime) {
	if (s_plugins != NULL) {
		PluginNode *node = s_plugins->FindNodeFromMime(mime);

		return (node != NULL) ? (FREE_IMAGE_FORMAT)node->m_id : FIF_UNKNOWN;
	}

	return FIF_UNKNOWN;
}

const char * DLL_CALLCONV
FreeImage_GetFormatFromFIF(FREE_IMAGE_FORMAT fif) {
	if (s_plugins != NULL) {
		PluginNode *node = s_plugins->FindNodeFromFIF(fif);

		return (node != NULL) ? (node->m_format != NULL) ? node->m_format : node->m_plugin->format_proc() : NULL;
	}

	return NULL;
}

const char * DLL_CALLCONV
FreeImage_GetFIFMimeType(FREE_IMAGE_FORMAT fif) {
	if (s_plugins != NULL) {
		PluginNode *node = s_plugins->FindNodeFromFIF(fif);

		return (node != NULL) ? (node->m_plugin != NULL) ? ( node->m_plugin->mime_proc != NULL )? node->m_plugin->mime_proc() : NULL : NULL : NULL;
	}

	return NULL;
}

const char * DLL_CALLCONV
FreeImage_GetFIFExtensionList(FREE_IMAGE_FORMAT fif) {
	if (s_plugins != NULL) {
		PluginNode *node = s_plugins->FindNodeFromFIF(fif);

		return (node != NULL) ? (node->m_extension != NULL) ? node->m_extension : (node->m_plugin->extension_proc != NULL) ? node->m_plugin->extension_proc() : NULL : NULL;
	}

	return NULL;
}

const char * DLL_CALLCONV
FreeImage_GetFIFDescription(FREE_IMAGE_FORMAT fif) {
	if (s_plugins != NULL) {
		PluginNode *node = s_plugins->FindNodeFromFIF(fif);

		return (node != NULL) ? (node->m_description != NULL) ? node->m_description : (node->m_plugin->description_proc != NULL) ? node->m_plugin->description_proc() : NULL : NULL;
	}

	return NULL;
}

const char * DLL_CALLCONV
FreeImage_GetFIFRegExpr(FREE_IMAGE_FORMAT fif) {
	if (s_plugins != NULL) {
		PluginNode *node = s_plugins->FindNodeFromFIF(fif);

		return (node != NULL) ? (node->m_regexpr != NULL) ? node->m_regexpr : (node->m_plugin->regexpr_proc != NULL) ? node->m_plugin->regexpr_proc() : NULL : NULL;
	}

	return NULL;
}

bool DLL_CALLCONV
FreeImage_FIFSupportsReading(FREE_IMAGE_FORMAT fif) {
	if (s_plugins != NULL) {
		PluginNode *node = s_plugins->FindNodeFromFIF(fif);

		return (node != NULL) ? node->m_plugin->load_proc != NULL : FALSE;
	}

	return FALSE;
}

bool DLL_CALLCONV
FreeImage_FIFSupportsWriting(FREE_IMAGE_FORMAT fif) {
	if (s_plugins != NULL) {
		PluginNode *node = s_plugins->FindNodeFromFIF(fif);

		return (node != NULL) ? node->m_plugin->save_proc != NULL : FALSE ;
	}

	return FALSE;
}

bool DLL_CALLCONV
FreeImage_FIFSupportsExportBPP(FREE_IMAGE_FORMAT fif, int depth) {
	if (s_plugins != NULL) {
		PluginNode *node = s_plugins->FindNodeFromFIF(fif);

		return (node != NULL) ?
			(node->m_plugin->supports_export_bpp_proc != NULL) ?
				node->m_plugin->supports_export_bpp_proc(depth) : FALSE : FALSE;
	}

	return FALSE;
}

bool DLL_CALLCONV
FreeImage_FIFSupportsExportType(FREE_IMAGE_FORMAT fif, FREE_IMAGE_TYPE type) {
	if (s_plugins != NULL) {
		PluginNode *node = s_plugins->FindNodeFromFIF(fif);

		return (node != NULL) ?
			(node->m_plugin->supports_export_type_proc != NULL) ?
				node->m_plugin->supports_export_type_proc(type) : FALSE : FALSE;
	}

	return FALSE;
}

bool DLL_CALLCONV
FreeImage_FIFSupportsICCProfiles(FREE_IMAGE_FORMAT fif) {
	if (s_plugins != NULL) {
		PluginNode *node = s_plugins->FindNodeFromFIF(fif);

		return (node != NULL) ?
			(node->m_plugin->supports_icc_profiles_proc != NULL) ?
				node->m_plugin->supports_icc_profiles_proc() : FALSE : FALSE;
	}

	return FALSE;
}

bool DLL_CALLCONV
FreeImage_FIFSupportsNoPixels(FREE_IMAGE_FORMAT fif) {
	if (s_plugins != NULL) {
		PluginNode *node = s_plugins->FindNodeFromFIF(fif);

		return (node != NULL) ?
			(node->m_plugin->supports_no_pixels_proc != NULL) ?
				node->m_plugin->supports_no_pixels_proc() : FALSE : FALSE;
	}

	return FALSE;
}

FREE_IMAGE_FORMAT DLL_CALLCONV
FreeImage_GetFIFFromFilename(const char *filename) {
	if (filename != NULL) {
		const char *extension;

		// get the proper extension if we received a filename

		char *place = strrchr((char *)filename, '.');
		extension = (place != NULL) ? ++place : filename;

		// look for the extension in the plugin table

		for (int i = 0; i < FreeImage_GetFIFCount(); ++i) {

			if (s_plugins->FindNodeFromFIF(i)->m_enabled) {

				// compare the format id with the extension

				if (FreeImage_stricmp(FreeImage_GetFormatFromFIF((FREE_IMAGE_FORMAT)i), extension) == 0) {
					return (FREE_IMAGE_FORMAT)i;
				} else {
					// make a copy of the extension list and split it

					char *copy = (char *)malloc(strlen(FreeImage_GetFIFExtensionList((FREE_IMAGE_FORMAT)i)) + 1);
					memset(copy, 0, strlen(FreeImage_GetFIFExtensionList((FREE_IMAGE_FORMAT)i)) + 1);
					memcpy(copy, FreeImage_GetFIFExtensionList((FREE_IMAGE_FORMAT)i), strlen(FreeImage_GetFIFExtensionList((FREE_IMAGE_FORMAT)i)));

					// get the first token

					char *token = strtok(copy, ",");

					while (token != NULL) {
						if (FreeImage_stricmp(token, extension) == 0) {
							free(copy);

								return (FREE_IMAGE_FORMAT)i;
						}

						token = strtok(NULL, ",");
					}

					// free the copy of the extension list

					free(copy);
				}
			}
		}
	}

	return FIF_UNKNOWN;
}

FREE_IMAGE_FORMAT DLL_CALLCONV
FreeImage_GetFIFFromFilenameU(const wchar_t *filename) {
#ifdef _WIN32
	if (filename == NULL) return FIF_UNKNOWN;

	// get the proper extension if we received a filename
	wchar_t *place = wcsrchr((wchar_t *)filename, '.');
	if (place == NULL) return FIF_UNKNOWN;
	// convert to single character - no national chars in extensions
	char *extension = (char *)malloc(wcslen(place)+1);
	unsigned int i=0;
	for(; i < wcslen(place); i++) // convert 16-bit to 8-bit
		extension[i] = (char)(place[i] & 0x00FF);
	// set terminating 0
	extension[i]=0;
	FREE_IMAGE_FORMAT fRet = FreeImage_GetFIFFromFilename(extension);
	free(extension);

	return fRet;
#else
	return FIF_UNKNOWN;
#endif // _WIN32
}

bool DLL_CALLCONV
FreeImage_ValidateFIF(FREE_IMAGE_FORMAT fif, FreeImageIO *io, fi_handle handle) {
	if (s_plugins != NULL) {
		bool validated = FALSE;

		PluginNode *node = s_plugins->FindNodeFromFIF(fif);

		if (node) {
			long tell = io->tell_proc(handle);

			validated = (node != NULL) ? (node->m_enabled) ? (node->m_plugin->validate_proc != NULL) ? node->m_plugin->validate_proc(io, handle) : FALSE : FALSE : FALSE;

			io->seek_proc(handle, tell, SEEK_SET);
		}

		return validated;
	}

	return FALSE;
}
