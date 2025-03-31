#include <iostream>
#include <exception>
#include <string>

#include <FreeImage.h>

#define CHECK(c) check(c, #c, __FILE__, __LINE__)

void check(bool condition, const char* message, const char* file = __FILE__, int line = __LINE__)
{
    if (condition == false)
    {
        auto str = std::string(file) + "(" + std::to_string(line) + "):" + message;
        std::cerr << str;
        throw std::runtime_error(str);
    }
}

int main(int argc, char* argv[])
{
    // Asset folder as unique argument
    check(argc >= 2, "Asset directory must be provided");
    std::string asset_dir = argv[1];

    FreeImage_Initialise();
    FreeImage_SetOutputMessage([](FREE_IMAGE_FORMAT fif, const char* message) {
        std::string fif_name = fif != FIF_UNKNOWN ? FreeImage_GetFormatFromFIF(fif) : "UNKNOWN";
        std::string err = std::string(message) + "(fif == " + fif_name + ")";
        check(false, message, __FILE__, __LINE__);
    });

    int iWidth = 16, iHeight = 16;
    // Allocate
    auto* hBitmap = FreeImage_Allocate(iWidth, iHeight, 32);
    CHECK(hBitmap != nullptr);
    FreeImage_Unload(hBitmap);

    // AllocateT
    hBitmap = FreeImage_AllocateT(FIT_FLOAT, iWidth, iHeight);
    CHECK(hBitmap != nullptr);

    // Clone
    FIBITMAP* hBitmap2 = FreeImage_Clone(hBitmap);
    CHECK(hBitmap2 != nullptr);
    FreeImage_Unload(hBitmap2);
    FreeImage_Unload(hBitmap);

    // Load
    std::string infile = asset_dir + "/import.png";	
    auto fif = FreeImage_GetFIFFromFilename(infile.c_str());
    CHECK(fif == FIF_PNG);
    hBitmap = FreeImage_Load(FIF_PNG, infile.c_str());
    CHECK(hBitmap != nullptr);

    // Save
    std::string outfile = "output.jpeg";
    int iReturn = FreeImage_Save(FIF_JPEG, hBitmap, outfile.c_str());
    CHECK(iReturn != 0);

    FreeImage_Unload(hBitmap);

    FreeImage_DeInitialise();
} 