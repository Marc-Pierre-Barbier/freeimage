// TestFreeImage.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>

#include <FreeImage.h>

// Read the input image file
// Get its main values (bpp, witdth, height...)
// Compare these values to the expected values read from a txt file
// Save the image in a new file
// Open the saved file
// Get its main values (bpp, witdth, height...)
// Compare these values to the expected values
static int ConvertImage(wchar_t* szInputImagePath, wchar_t* szOutputImagePath)
{
	int iRet = 0;

	FreeImage_Initialise();

	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	FIBITMAP* pFIDib = nullptr;

	// Check the file signature and deduce its format
	fif = FreeImage_GetFileTypeU(szInputImagePath, 0);

	if (fif == FIF_UNKNOWN)
	{
		// No signature: try to guess the file format from the file extension
		fif = FreeImage_GetFIFFromFilenameU(szInputImagePath);
	}

	// Check that the plugin has reading capabilities ...
	if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif))
	{
		int iFlag = 0; // default parameters for all formats...

		// Best quality load for jpeg format
		if (fif == FIF_JPEG)
			iFlag = JPEG_QUALITYSUPERB;

		// Filenames conversion
		size_t szNumOfCharConv;
		char* szInput = new char[_MAX_PATH + 1];
		memset(szInput, 0, _MAX_PATH + 1);
		wcstombs_s(&szNumOfCharConv, szInput, _MAX_PATH*sizeof(char), szInputImagePath, _MAX_PATH * sizeof(char));
		char* szOutput = new char[_MAX_PATH + 1];
		memset(szOutput, 0, _MAX_PATH + 1);
		wcstombs_s(&szNumOfCharConv, szOutput, _MAX_PATH * sizeof(char), szOutputImagePath, _MAX_PATH * sizeof(char));

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
					std::wcout << L"Error when comparing expected values of " << szOutputImagePath << std::endl;
					iRet = 1;
				}
			}
			else
			{
				std::wcout << L"Error when comparing expected values of " << szInputImagePath << std::endl;
				iRet = 1;
			}
		}
		else
		{
			std::wcout << L"Error when opening " << szInputImagePath << std::endl;
			iRet = 1;
		}
	}
	else
	{
		std::wcout << L"Format not supported: " << szInputImagePath << std::endl;
		iRet = 1;
	}

	FreeImage_DeInitialise();

	return iRet;
}



// Usage: TestFreeImage -in <input image file> -out <output image file>
int wmain(int argc, wchar_t* argv[])
{
	wchar_t* szInputImagePath	= nullptr;
	wchar_t* szOutputImagePath	= nullptr;

	if (argc == 5)
	{
		while (--argc > 0)
		{
			if ((*++argv)[0] == '-')
			{
				if (!wcscmp(*argv, L"-in"))
				{
					// Check if there is something after the option on the command line
					if (*++argv)
					{
						szInputImagePath = *argv;
					}
					else
					{
						std::wcout << L"Error: -in option needs an input image filename parameter." << std::endl;
						return true;
					}

					argc--;
				}

				if (!wcscmp(*argv, L"-out"))
				{
					// Check if there is something after the option on the command line
					if (*++argv)
					{
						szOutputImagePath = *argv;
					}
					else
					{
						std::wcout << L"Error: -out option needs an output image filename parameter." << std::endl;
						return true;
					}

					argc--;
				}
			}
		}
	}

	if (szInputImagePath != nullptr && szOutputImagePath != nullptr)
	{
		std::wcout << L"Input:  " << szInputImagePath << std::endl;

		int ret = ConvertImage(szInputImagePath, szOutputImagePath);
		if (ret == 0)
			std::wcout << L"Output: " << szOutputImagePath << std::endl;
		else
			std::wcout << L"Failed to genrate output" << std::endl;
		return ret;
	}
	else
		std::wcout << L"Usage: TestFreeImage -in <Input image filename> -out <Output image filename>" << std::endl;
}

