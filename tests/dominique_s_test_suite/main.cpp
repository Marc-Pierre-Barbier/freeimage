// TestFreeImage.cpp : This file contains the 'main' function. Program execution begins and ends there.
//



//quick and dirty windows/linux compatiblity
#ifdef __linux__
#include <linux/limits.h>
#define MAX_PATH PATH_MAX
#define CHAR_TYPE char
#define CHAR_STRCMP strcmp
#define Fre_GetFileType FreeImage_GetFileType
#define Fre_GetFIFFromFilename FreeImage_GetFIFFromFilename
#define CHAR_OUT cout

#define ERR_COMP "Error when comparing expected values of "
#define ERR_OPENING "Error when opening "
#define ERR_FORMAT "Format not supported: "
#define STR_IN "-in"
#define STR_OUT "-out"
#define STR_USAGE "Usage: TestFreeImage -in <Input image filename> -out <Output image filename>"
#define STR_OUTPUT_ERR "Failed to genrate output"
#define STR_OUTPUT "Output: "
#define STR_INPUT "Input:  "
#define STR_ERR_INPUT "Error: -in option needs an input image filename parameter."
#define STR_ERR_OUTPUT "Error: -out option needs an output image filename parameter."

#else
#define CHAR_TYPE wchar_t
#define CHAR_STRCMP wcscmp
#define Fre_GetFileType FreeImage_GetFileTypeU
#define Fre_GetFIFFromFilename FreeImage_GetFIFFromFilenameU
#define CHAR_OUT wcout

#define ERR_COMP L"Error when comparing expected values of "
#define ERR_OPENING L"Error when opening "
#define ERR_FORMAT L"Format not supported: "
#define STR_IN L"-in"
#define STR_OUT L"-out"
#define STR_USAGE L"Usage: TestFreeImage -in <Input image filename> -out <Output image filename>"
#define STR_OUTPUT_ERR L"Failed to genrate output"
#define STR_OUTPUT L"Output: "
#define STR_INPUT L"Input:  "
#define STR_ERR_INPUT L"Error: -in option needs an input image filename parameter."
#define STR_ERR_OUTPUT L"Error: -out option needs an output image filename parameter."

#include <windows.h>
#include <windef.h>
#endif

#include <iostream>
#include <fstream>
#include <cstring>
#include <FreeImage.h>

// Read the input image file
// Get its main values (bpp, witdth, height...)
// Compare these values to the expected values read from a txt file
// Save the image in a new file
// Open the saved file
// Get its main values (bpp, witdth, height...)
// Compare these values to the expected values
static int ConvertImage(CHAR_TYPE* szInputImagePath, CHAR_TYPE* szOutputImagePath)
{
	int iRet = 0;

	FreeImage_Initialise();

	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	FIBITMAP* pFIDib = nullptr;

	// Check the file signature and deduce its format
	fif = Fre_GetFileType(szInputImagePath, 0);

	if (fif == FIF_UNKNOWN)
	{
		// No signature: try to guess the file format from the file extension
		fif = Fre_GetFIFFromFilename(szInputImagePath);
	}

	// Check that the plugin has reading capabilities ...
	if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif))
	{
		int iFlag = 0; // default parameters for all formats...

		// Best quality load for jpeg format
		if (fif == FIF_JPEG)
			iFlag = JPEG_QUALITYSUPERB;

		// Filenames conversion
		#ifdef __unix__
		char* szInput = szInputImagePath;
		char* szOutput = szOutputImagePath;
		#else
		size_t szNumOfCharConv;
		char* szInput = new char[MAX_PATH + 1];
		memset(szInput, 0, MAX_PATH + 1);
		szNumOfCharConv = wcstombs(szInput, szInputImagePath, MAX_PATH * sizeof(char));
		char* szOutput = new char[MAX_PATH + 1];
		memset(szOutput, 0, MAX_PATH + 1);
		szNumOfCharConv = wcstombs(szOutput, szOutputImagePath, MAX_PATH * sizeof(char));
		#endif

		// Load the file
		pFIDib = FreeImage_Load(fif, szInput, iFlag);

		// Save the file
		if (pFIDib != nullptr)
		{
			// Get main values
			unsigned int uiBPP		= FreeImage_GetBPP(pFIDib);
			unsigned int uiWidth	= FreeImage_GetWidth(pFIDib);
			unsigned int uiHeight	= FreeImage_GetHeight(pFIDib);
			unsigned int uiLine		= FreeImage_GetLine(pFIDib);
			unsigned int uiPitch	= FreeImage_GetPitch(pFIDib);

			// Read and compare expected values
			unsigned int uiBPP_Read;
			unsigned int uiWidth_Read;
			unsigned int uiHeight_Read;
			unsigned int uiLine_Read;
			unsigned int uiPitch_Read;

			std::string ifsInput = szInput;
			ifsInput += ".txt";
			std::ifstream ifs(ifsInput);
			ifs >> uiBPP_Read;
			ifs >> uiWidth_Read;
			ifs >> uiHeight_Read;
			ifs >> uiLine_Read;
			ifs >> uiPitch_Read;

			if (uiBPP == uiBPP_Read && uiWidth == uiWidth_Read && uiHeight == uiHeight_Read && uiLine == uiLine_Read && uiPitch == uiPitch_Read)
			{
				// Save the dib data in a new file
				FreeImage_Save(fif, pFIDib, szOutput, iFlag);
				FreeImage_Unload(pFIDib);

				// Open the saved file
				pFIDib = FreeImage_Load(fif, szOutput, iFlag);

				// Read and compare expected values
				unsigned int uiBPP = FreeImage_GetBPP(pFIDib);
				unsigned int uiWidth = FreeImage_GetWidth(pFIDib);
				unsigned int uiHeight = FreeImage_GetHeight(pFIDib);
				unsigned int uiLine = FreeImage_GetLine(pFIDib);
				unsigned int uiPitch = FreeImage_GetPitch(pFIDib);

				if (uiBPP == uiBPP_Read && uiWidth == uiWidth_Read && uiHeight == uiHeight_Read && uiLine == uiLine_Read && uiPitch == uiPitch_Read)
				{
					iRet = 0;
				}
				else
				{
					std::CHAR_OUT << ERR_COMP << szOutputImagePath << std::endl;
					iRet = 1;
				}
			}
			else
			{
				std::CHAR_OUT << ERR_COMP << szInputImagePath << std::endl;
				iRet = 1;
			}
		}
		else
		{
			std::CHAR_OUT << ERR_OPENING << szInputImagePath << std::endl;
			iRet = 1;
		}
	}
	else
	{
		std::CHAR_OUT << ERR_FORMAT << szInputImagePath << std::endl;
		iRet = 1;
	}

	FreeImage_DeInitialise();

	return iRet;
}



// Usage: TestFreeImage -in <input image file> -out <output image file>
#ifdef __unix__
int main(int argc, char * argv[])
{
#else
int wmain(int argc, wchar_t* argv[])
{
#endif
	CHAR_TYPE* szInputImagePath	= nullptr;
	CHAR_TYPE* szOutputImagePath = nullptr;

	if (argc == 5)
	{
		while (--argc > 0)
		{
			if ((*++argv)[0] == '-')
			{
				if (!CHAR_STRCMP(*argv, STR_IN))
				{
					// Check if there is something after the option on the command line
					if (*++argv)
					{
						szInputImagePath = *argv;
					}
					else
					{
						std::CHAR_OUT << STR_ERR_INPUT << std::endl;
						return true;
					}

					argc--;
				}

				if (!CHAR_STRCMP(*argv, STR_OUT))
				{
					// Check if there is something after the option on the command line
					if (*++argv)
					{
						szOutputImagePath = *argv;
					}
					else
					{
						std::CHAR_OUT << STR_ERR_OUTPUT << std::endl;
						return true;
					}

					argc--;
				}
			}
		}
	}

	if (szInputImagePath != nullptr && szOutputImagePath != nullptr)
	{
		std::CHAR_OUT << STR_INPUT << szInputImagePath << std::endl;

		int ret = ConvertImage(szInputImagePath, szOutputImagePath);
		if (ret == 0)
			std::CHAR_OUT << STR_OUTPUT << szOutputImagePath << std::endl;
		else
			std::CHAR_OUT << STR_OUTPUT_ERR << std::endl;
		return ret;
	}
	else
		std::CHAR_OUT << STR_USAGE << std::endl;
}

