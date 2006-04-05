#include "helper.hpp"

void Helper::stringToKey(char                  source_string[BYTES_PER_KEY],
                 sc_bv<BITS_PER_KEY> & key)
{
  for (int index = 0; index < BYTES_PER_KEY; index++)
  {
    sc_bv<8> one_byte = static_cast< sc_uint<BITS_PER_BYTE> >(source_string[index]);
    key.range(BITS_PER_BYTE * index + MOST_SIGNIFICANT_BIT,
              BITS_PER_BYTE * index + LEAST_SIGNIFICANT_BIT)
      = one_byte;
  }
}

void Helper::stringToKeys(char *        source_string,
                  VanillaKeys & keys)
{
  int length = strlen(source_string);
  for (int key_nr = 0; key_nr < MAX_NUMBER_OF_KEYS; key_nr++)
  {
    char key_chars[BYTES_PER_KEY];
    // may get negative
    int remaining_chars = length - key_nr * BYTES_PER_KEY;
    if (remaining_chars > 0)
    {
      // does zero-padding itself
      strncpy(key_chars, (const char *) &source_string[key_nr * BYTES_PER_KEY], BYTES_PER_KEY);
      keys.byte_usage[key_nr] = (BYTES_PER_KEY < remaining_chars ? BYTES_PER_KEY : remaining_chars);
    }
    else
    {
      for (int index = 0; index < BYTES_PER_KEY; index++)
      {
        key_chars[index] = '\0';
      }
      keys.byte_usage[key_nr] = 0;
    }
    stringToKey(key_chars, keys.bits[key_nr]);
  }
}

static void Helper::stringToDataword(Datachars                  &source_string,
                              int                               length,
                              sc_bv<BITS_PER_DATAWORD>          &dest_dataword)
{
  for (int index = 0; index < length; index++)
  {
    dest_dataword.range(BITS_PER_BYTE * index + MOST_SIGNIFICANT_BIT,
                        BITS_PER_BYTE * index + LEAST_SIGNIFICANT_BIT)
      = static_cast< sc_uint<BITS_PER_BYTE> >(source_string.position[index]);
  }

  for(int index = BYTES_PER_DATAWORD-1; index >= length; index--)
  {
    dest_dataword.range(BITS_PER_BYTE * index + MOST_SIGNIFICANT_BIT,
                        BITS_PER_BYTE * index + LEAST_SIGNIGICANT_BIT)
      = 0;
  }
}

void Helper::datawordToString(sc_bv<BITS_PER_DATAWORD>   source_dataword,
                              Datachars                & dest_string)
{
  for (int index = 0; index < BYTES_PER_DATAWORD; index++)
  {
    sc_bv<8> one_byte = source_dataword.range(BITS_PER_BYTE * index + MOST_SIGNIFICANT_BIT,
                                              BITS_PER_BYTE * index + LEAST_SIGNIFICANT_BIT);
    dest_string.position[index] = static_cast< sc_uint<BITS_PER_BYTE> >(one_byte);
  }

}

