#include <iostream>
#include <string>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <sstream>
#include <array>

#include "BundleReader.hpp"
#include "TextureResolver.hpp"

#include "xbg.h"
#include "core.h"

int GetVersionByMagic(const std::string& file, std::unique_ptr<std::byte[]>& buffer, int* size)
{
	std::array<uint8_t, 4> bundle_magic{ 0x62, 0x75, 0x72, 0x47 };
	std::array<uint8_t, 4> xds_offset_v1{ 0x00, 0x00, 0x00, 0x03 };
	std::array<uint8_t, 4> xds_offset_v2{ 0x00, 0x00, 0x00, 0x01 };

	if (!std::filesystem::exists(file))
	{
		return -1;
	}

	std::ifstream stream = std::ifstream(file, std::ios::binary);

	if (!stream.is_open())
	{
		return -1;
	}

	std::array<uint8_t, 4> stream_magic_v1{};
	stream.read(reinterpret_cast<char*>(stream_magic_v1.data()), 4);

	std::array<uint8_t, 4> stream_magic_v2{};
	stream.read(reinterpret_cast<char*>(stream_magic_v2.data()), 4);

	stream.seekg(0, std::ios::end);

	if (std::memcmp(bundle_magic.data(), stream_magic_v1.data(), 4) == 0)
	{
		*size = static_cast<int>(stream.tellg());
		buffer = std::make_unique<std::byte[]>(*size);
		stream.seekg(0, std::ios::beg);

		stream.read(reinterpret_cast<char*>(buffer.get()), *size);
		stream.close();

		return 1;
	};

	if (std::memcmp(xds_offset_v1.data(), stream_magic_v1.data(), 4) == 0 && std::memcmp(xds_offset_v2.data(), stream_magic_v2.data(), 4) == 0)
	{
		*size = static_cast<int>(stream.tellg());
		buffer = std::make_unique<std::byte[]>((*size) + 0x30); // reserve memory to xds header
		stream.seekg(0, std::ios::beg);

		std::fill(buffer.get(), buffer.get() + 0x30, std::byte{0});

		stream.read(reinterpret_cast<char*>(buffer.get()) + 0x30, *size);
		stream.close();

		return 2;
	};

	return 0;
}

int main(int argc, char* args[])
{
	const char* VERSION = "1.3.1";

	if (argc > 1)
	{
		for (size_t i = 1; i < argc; ++i)
		{
			std::string file_input(args[i]);

			if (!std::filesystem::exists(file_input))
			{
				std::cout << "ERROR: Failed to proceed, the file on input does not exist.\n\n";

				return 1;
			}

			std::unique_ptr<std::byte[]> buffer = nullptr;
			int size = 0;

			switch (GetVersionByMagic(file_input, buffer, &size))
			{
			case 0:
			{
				std::cout << "Unsupported file extension: " << std::filesystem::path(file_input).extension() << "\n";
				break;
			}
			case 1: // file_extension == ".swatchbin"
			{
				std::cout << "ForzaTech Graphics Converter v" << VERSION << "-release+" << GetCoreBuild() << "\n";

				std::filesystem::path file_output = std::filesystem::path(file_input).replace_extension(".dds");

				auto reader = std::make_unique<fmnext::BundleReader>(buffer.get(), size);
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
				std::cout << "Forza Graphics Converter v" << VERSION << "-release+" << GetCoreBuild() << "\n";

				std::filesystem::path file_output = std::filesystem::path(file_input).replace_extension(".tga");

				if (CreateXDSBuffer(buffer.get(), &size) == S_OK)
				{
					Convert(buffer.get(), size, file_output.string().c_str());
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