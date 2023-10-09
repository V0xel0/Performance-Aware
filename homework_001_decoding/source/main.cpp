#include <fstream>
#include <filesystem>
#include <iostream>
#include <cassert>

#include "Utils.hpp"

int main()
{
	byte *memory = nullptr;
	static const char* reg_lut[2][8] = { { "AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH" },
								  { "AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI" } };
	
	std::filesystem::path file_path {"../input/listing_0038_many_register_mov"}; 
	std::ifstream file(file_path, std::ios::binary);
	
	if (file.is_open())
	{
		auto file_size = std::filesystem::file_size(file_path);
		assert(file_size >= 2 && "File is too small!");
		memory = (byte *)malloc(file_size);
		file.read((char *)memory, file_size);
		
		for(u32 i=0; i<file_size; i+=2)
		{
			byte first_byte = memory[i];
			byte second_byte = memory[i + 1];
			
			byte instruction = first_byte >> 2;
			if (!(instruction ^ 0b00100010))
			{
				byte w = TestBit(first_byte, 0);
				byte d = TestBit(first_byte, 1);
				byte mod = (second_byte >> 6);
				byte reg = (second_byte >> 3) & 0b00000111;
				byte rm  = (second_byte) & 0b00000111;
				
				const char *reg_a = reg_lut[w][reg];
				const char *reg_b = reg_lut[w][rm];
				
				if (d)
					std::cout << "mov " << reg_a << " " << reg_b << "\n";
				else
					std::cout << "mov " << reg_b << " " << reg_a << "\n";
			}
		}
	}
}