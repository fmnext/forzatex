#include <iostream>
#include <string>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <sstream>

#include "BundleReader.hpp"
#include "TextureResolver.hpp"

#include "xbg.h"
#include "core.h"

int GetVersionByMagic(const std::string& file)
{
	char bundle_magic[4] = { 0x62, 0x75, 0x72, 0x47 };
	char xds_offset_v1[4] = { 0x00, 0x00, 0x00, 0x03 };
	char xds_offset_v2[4] = { 0x00, 0x00, 0x00, 0x01 };

	if (!std::filesystem::exists(file))
	{
		return -1;
	}

	std::ifstream stream = std::ifstream(file, std::ios::binary);

	if (!stream.is_open())
	{
		return -1;
	}

	char stream_magic_v1[4];
	stream.read(reinterpret_cast<char*>(&stream_magic_v1), 4);

	char stream_magic_v2[4];
	stream.read(reinterpret_cast<char*>(&stream_magic_v2), 4);

	if (std::memcmp(bundle_magic, stream_magic_v1, 4) == 0)
	{
		return 1;
	};

	if (std::memcmp(xds_offset_v1, stream_magic_v1, 4) == 0 && std::memcmp(xds_offset_v2, stream_magic_v2, 4) == 0)
	{
		return 2;
	};

	return 0;
}


int main(int argc, char* args[])
{
	const char* VERSION = "1.3.0";

	if (argc > 1)
	{
		for (size_t i = 1; i < argc; ++i)
		{
			std::string file_input(args[i]);
			std::filesystem::path file_extension = std::filesystem::path(file_input).extension();


			if (!std::filesystem::exists(file_input))
			{
				std::cout << "ERROR: Failed to proceed, the file on input does not exist.\n\n";

				return 1;
			}

			switch (GetVersionByMagic(file_input))
			{
			case 0:
			{
				std::cout << "Unsupported file extension: " << file_extension << "\n";
				break;
			}
			case 1: // file_extension == ".swatchbin"
			{
				std::cout << "ForzaTech Graphics Converter v" << VERSION << "-release+" << GetCoreVersion() << "\n";

				std::filesystem::path file_output = std::filesystem::path(file_input).replace_extension(".dds");

				auto reader = std::make_unique<fmnext::BundleReader>(file_input);
				if (reader->Init())
				{
					if (!reader->bundle.Textures.empty())
					{
						auto texture_instance = fmnext::TextureResolver(reader->bundle);
						texture_instance.SaveToDDSFile(file_output.string());
						//texture_instance.SaveToPNGFile(file_output.string());

						std::cout << "File saved to " << file_output.string() << " \n";
					}

					if (reader->bundle.Textures.empty())
					{
						std::cout << "ERROR: Bundle does not contains any graphics data. \n";
					}
				}

				break;
			}
			case 2: //file_extension == ".xds"
			{
				std::cout << "Forza Graphics Converter v" << VERSION << "-release+" << GetCoreVersion() << "\n";

				HRESULT res = ReadXDSFile(file_input.c_str());
				if (res != S_OK)
				{
					//return 0;
				}

				break;
			}
			default:
			{
				// unreachable
				break;
			}
			}
		}
	}
	else 
	{
		std::cout << "Usage: " << args[0] << " filename.swatchbin/xds" << "\n";
	} 

	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
