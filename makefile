all:
	gcc -o argon2crypt *.c lib/*.c -lsodium

clean:
	rm argon2crypt
