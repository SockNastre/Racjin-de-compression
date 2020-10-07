#include <vector>

std::vector<uint8_t> compress(const std::vector<uint8_t>& buffer) {  //LZSS based 

	uint32_t index = 0; //position of an element from the input buffer

	uint8_t lastEncByte = 0;//last encoded byte

	uint8_t  bitShift = 0; //shift by bitShift (used to fold tokens)

	std::vector<uint32_t> frequencies(256, 0);

	std::vector<uint32_t> seqIndices(8192, 0);

	std::vector<uint16_t> tokens;

	std::vector<uint8_t> compressedBuffer;

	tokens.reserve(buffer.size() / 2);

	compressedBuffer.reserve(buffer.size());


	while (index < buffer.size()) {

		uint8_t bestFreq = 0;

		uint8_t bestMatch = 0;

		//To get the exact same compression for Bomberman Kart DX use:
		//if (frequencies[lastEncByte] == 256) frequencies[lastEncByte] = 0x00;

		uint8_t positionsToCheck = frequencies[lastEncByte] < 32 ? (frequencies[lastEncByte] & 0x1F) : 32;

		uint32_t seqIndex = index;

		for (uint8_t freq = 0; freq < positionsToCheck; freq++) {

			uint16_t key = freq + lastEncByte * 32; //0x1F + 0xFF*32 = 8191

			uint32_t srcIndex = seqIndices[key];

			uint8_t matched = 0;

			uint8_t maxLength = index + 8 < buffer.size() ? 8 : buffer.size() - index;

			for (uint8_t offset = 0; offset < maxLength; ++offset) {
				if (buffer[srcIndex + offset] == buffer[index + offset]) ++matched;
				else break;
			}

			if (matched > bestMatch) {
				bestFreq = freq;
				bestMatch = matched;
			}

		}

		uint16_t token = 0x00;

		if (bestMatch > 0) { //found a better match?
			token = token | (bestFreq << 3); //f|ooooolll //f=0 (flag), o - occurrences/frequency, l -length
			token = token | (bestMatch - 1);   //encode a reference
			index += bestMatch;
		}
		else { //encode byte literal
			token = 0x100 | buffer[index]; //f|bbbbbbbb //f=1
			++index;
		}

		token = token << bitShift; //prepare for folding

		tokens.push_back(token);

		++bitShift;

		if (bitShift == 8) bitShift = 0;

		uint16_t key = (frequencies[lastEncByte] & 0x1F) + lastEncByte * 32; //0x1F + 0xFF*32 = 8191

		seqIndices[key] = seqIndex;

		frequencies[lastEncByte] = frequencies[lastEncByte] + 1; //increase by 1 (up to 31)

		lastEncByte = buffer[index - 1];

	}

	//Fold tokens (8 tokens, 16 bytes -> 8 tokens, 9 bytes)
	for (uint32_t i = 0; i < tokens.size(); i = i + 8) {

		uint8_t groupSize = i + 8 < tokens.size() ? 8 : tokens.size() - i;

		for (uint8_t s = 0; s <= groupSize; s += 2) {

			uint16_t first = s > 0 ? tokens[s + i - 1] : 0x00;
			uint16_t middle = s < groupSize ? tokens[s + i] : 0x00;
			uint16_t last = s < groupSize - 1 ? tokens[s + i + 1] : 0x00;

			uint16_t result = middle | (first >> 8) | (last << 8);

			compressedBuffer.push_back(result & 0xFF);

			if (s < groupSize) compressedBuffer.push_back(result >> 8);


		}



	}


	return compressedBuffer;

}




