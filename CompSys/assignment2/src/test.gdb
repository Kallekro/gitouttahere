start

### Priting different representations of number. 

# Decimal
# to hexadecimal
p/x 192
# to binary
p/t 192 

#Hexadecimal
# to binary
p/t 0x80
# to decimal
p/d 0x80 

##Binary
# to hexadecimal
p/x 0b110
# to decimal
p/d 0b110 


p "Some basic tests.."
p UTF8_CONT(128) != 0
p UTF8_2B(192) != 0
p UTF8_3B(224) != 0
p UTF8_4B(240) != 0

p "Testing UTF8_CONT.."
p UTF8_CONT(128 + 1) != 0
p UTF8_CONT(128 | 1) != 0
p UTF8_CONT(128 | 63) != 0
p UTF8_CONT(128 | 63) > 0
p UTF8_CONT(128 + 64) == 0
p UTF8_CONT(128 | 64) == 0

p "Testing UTF8_2B.."
p UTF8_2B(128 + 64) != 0
p UTF8_2B(128 | 64) != 0
p UTF8_2B(128 | 64 | 31) != 0
p UTF8_2B(128 | 64 | 31) > 0
p UTF8_2B(128 + 64 + 32) == 0 
p UTF8_2B(128 | 64 | 32) == 0
p 

p "Testing UTF8_3B.."
p UTF8_3B(128 + 64 + 32) != 0
p UTF8_3B(128 | 64 | 32) != 0
p UTF8_3B(128 | 64 | 32 | 15) != 0 
p UTF8_3B(128 | 64 | 32 | 15) > 0
p UTF8_3B(128 + 64 + 32 + 16) == 0
p UTF8_3B(128 | 64 | 32 | 16) == 0
p 
p "Testing UTF8_4B.."
p UTF8_4B(128 + 64 + 32 + 16) != 0
p UTF8_4B(128 | 64 | 32 | 16) != 0
p UTF8_4B(128 | 64 | 32 | 16) != 0
p UTF8_4B(128 | 64 | 32 | 16 | 7) > 0
p UTF8_4B(128 + 64 + 32 + 16 + 8) == 0
p UTF8_4B(128 | 64 | 32 | 16 | 8) == 0

p "Custom tests"
p UTF8_2B(0b11110000) != 0
p UTF8_2B(0b11100000) != 0
p UTF8_2B(0b11000100) != 1
p UTF8_2B(0b10000000) != 0

p UTF8_3B(0b11110000) != 0
p UTF8_3B(0b11100000) != 1
p UTF8_3B(0b11000100) != 0
p UTF8_3B(0b10000000) != 0

p UTF8_4B(0b11110000) != 1
p UTF8_4B(0b11100000) != 0
p UTF8_4B(0b11000100) != 0
p UTF8_4B(0b10000000) != 0

p "Test with only 1's and only 0's"
p UTF8_4B(256) == 0
p UTF8_4B(0) == 0
p UTF8_3B(256) == 0
p UTF8_3B(0) == 0
p UTF8_2B(256) == 0
p UTF8_2B(0) == 0
p UTF8_CONT(256) == 0
p UTF8_CONT(0) == 0



q
