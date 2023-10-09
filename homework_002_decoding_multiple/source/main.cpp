#include <fstream>
#include <string>
#include <format>
#include <filesystem>
#include <iostream>
#include <cassert>
#include "Utils.hpp"

int main()
{
	byte *memory = nullptr;
	static const char *reg_lut[2][8] = { { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" },
								  { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di" } };
	
	static const char *rm_mod_lut[8] = { "bx + si", "bx + di", "bp + si", 
										 "bp + di", "si", "di", "bp", "bx" };
	u64 file_size = 0;
	std::filesystem::path file_path {"../input/listing_0040_challenge_movs"}; 
	std::ifstream file(file_path, std::ios::binary);
	std::string out_buf{};
	out_buf.reserve(32);
	std::string scratch{};
	scratch.reserve(32);
	
	if (file.is_open())
	{
		file_size = std::filesystem::file_size(file_path);
		assert(file_size >= 2 && "File to small, minimum is 2 bytes!");
		memory = (byte *)malloc(file_size);
		file.read((char *)memory, file_size);
		file.close();
	}
	
	u32 offset = 0;
	while(offset < file_size)
	{
		byte first_byte = memory[offset];
		byte second_byte = memory[offset + 1];
		u32 instr_size = 2;
		
		// Below data might not be valid for every opcode
		byte w = TestBit(first_byte, 0);
		byte d = TestBit(first_byte, 1);
		byte mod = (second_byte >> 6);
		byte reg = (second_byte >> 3) & 0b00000111;
		byte rm  = (second_byte) & 0b00000111;

		if (((first_byte & 0b11111100) == 0b10001000) // reg/mem to/from reg
		    || ((first_byte & 0b11111110) == 0b11000110)) // also immediate to reg/mem
		{
			out_buf += "mov ";
			instr_size = mod == 3 ? 2 : mod + 2;
			const char *const part_reg = reg_lut[w][reg];
			scratch = reg_lut[w][rm];
			byte imm_data_offset = 0;
	
			if (mod == 2)
			{
				scratch = std::format("[{} + {}]", rm_mod_lut[rm], *(s16 *)(memory + offset + 2));
				imm_data_offset = 4;
			}
			else if (mod == 1)
			{
				scratch = std::format("[{} + {}]", rm_mod_lut[rm], *(s8 *)(memory + offset + 2));
				imm_data_offset = 3;
			}
			else if (mod == 0)
			{
				if (rm == 6)
				{
					scratch = std::format("[{}]", *(s16 *)(memory + offset + 2));
					instr_size = 4;
					imm_data_offset = 4;
				}
				else
				{
					scratch = std::format("[{}]", rm_mod_lut[rm]);
					imm_data_offset = 2;
				}
			}
			
			if (d && ((first_byte & 0b11111110) == 0b11000110)) // handle immediate as specific case
			{
				if (w)
				{
					instr_size = imm_data_offset + 2;
					out_buf += std::format("{}, word {}\n", scratch, *(s16 *)(memory + offset + imm_data_offset));
				}
				else
				{
					instr_size = imm_data_offset + 1;
					out_buf += std::format("{}, byte {}\n", scratch, *(s8 *)(memory + offset + imm_data_offset));
				}
			}
			else if (d)
				out_buf += std::format("{}, {}\n", part_reg, scratch);
			else
				out_buf += std::format("{}, {}\n", scratch, part_reg);
		}
		else if ((first_byte & 0b11110000) == 0b10110000) // immediate to reg/mem
		{
			out_buf += "mov ";
			w = TestBit(first_byte, 3);
			instr_size = w + 2;
			reg  = (first_byte) & 0b00000111;
			const char *const part_reg = reg_lut[w][reg];
			
			if (w == 1)
				scratch = std::format("{}", *(s16 *)(memory + offset + 1));
			else
				scratch = std::format("{}", *(s8 *)(memory + offset + 1));
		
			out_buf += std::format("{}, {}\n", part_reg, scratch);
		}
		else if ((first_byte & 0b11100000) == 0b10100000) // mem to acc and acc to mem
		{
			instr_size = 2 + w;
			out_buf += "mov ";
			const char *const part_reg = reg_lut[w][0];
			
			if (w)
				scratch = std::format("[{}]", *(s16 *)(memory + offset + 1));
			else
			     scratch = std::format("[{}]", *(s8 *)(memory + offset + 1));
				          
			// The "d" bit case works other way around than in reg/mem to/from reg, cause FU
			if (d)
				out_buf += std::format("{}, {}\n", scratch, part_reg);
			else
				out_buf += std::format("{}, {}\n", part_reg, scratch);
		}
		
		std::cout << out_buf;
		out_buf.clear();
		scratch.clear();
		offset += instr_size;
	}
}