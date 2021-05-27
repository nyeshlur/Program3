/*
Program 3
Base code provided by rtdimpsey.
Modified by Nayana Yeshlur.

This program implements file i/o methods found in the Unix original stdio.h.
*/

#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <stdarg.h>   
#include <stdlib.h>   
#include "stdio.h" 
using namespace std;

char decimal[100];

int recursive_itoa(int arg) 
{
	int div = arg / 10;
	int mod = arg % 10;
	int index = 0;
	if (div > 0)
	{
		index = recursive_itoa(div);
	}
	decimal[index] = mod + '0';
	return ++index;
}

char *itoa(const int arg) 
{
	bzero(decimal, 100);
	int order = recursive_itoa(arg);
	char *new_decimal = new char[order + 1];
	bcopy(decimal, new_decimal, order + 1);
	return new_decimal;
}

int printf(const void *format, ...) 
{
	va_list list;
	va_start(list, format);

	char *msg = (char *)format;
	char buf[1024];
	int nWritten = 0;

	int i = 0, j = 0, k = 0;
	while (msg[i] != '\0') 
	{
		if (msg[i] == '%' && msg[i + 1] == 'd')
		{
			buf[j] = '\0';
			nWritten += write(1, buf, j);
			j = 0;
			i += 2;

			int int_val = va_arg(list, int);
			char *dec = itoa(abs(int_val));
			if (int_val < 0)
			{
				nWritten += write(1, "-", 1);
			}	
			nWritten += write(1, dec, strlen(dec));
			delete dec;
		}
		else
		{
			buf[j++] = msg[i++];
		}	
	}
	if (j > 0)
	{
		nWritten += write(1, buf, j);
	}	
	va_end( list );
	return nWritten;
}

int setvbuf(FILE *stream, char *buf, int mode, size_t size) 
{
	if (mode != _IONBF && mode != _IOLBF && mode != _IOFBF)
	{
		return -1;
	}	
	stream->mode = mode;
	stream->pos = 0;
	if (stream->buffer != (char *)0 && stream->bufown == true)
	{
		delete stream->buffer;
	}
	
	switch ( mode ) 
	{
		case _IONBF:
			stream->buffer = (char *)0;
			stream->size = 0;
			stream->bufown = false;
			break;
		case _IOLBF:
		case _IOFBF:
			if (buf != (char *)0) 
			{
				stream->buffer = buf;
				stream->size   = size;
				stream->bufown = false;
			}
			else 
			{
				stream->buffer = new char[BUFSIZ];
				stream->size = BUFSIZ;
				stream->bufown = true;
			}
			break;
	}
	return 0;
}

void setbuf(FILE *stream, char *buf) 
{
	setvbuf(stream, buf, ( buf != (char *)0 ) ? _IOFBF : _IONBF , BUFSIZ);
}

FILE *fopen(const char *path, const char *mode) 
{
	FILE *stream = new FILE();
	setvbuf(stream, (char *)0, _IOFBF, BUFSIZ);
	
	// fopen( ) mode
	// r or rb = O_RDONLY
	// w or wb = O_WRONLY | O_CREAT | O_TRUNC
	// a or ab = O_WRONLY | O_CREAT | O_APPEND
	// r+ or rb+ or r+b = O_RDWR
	// w+ or wb+ or w+b = O_RDWR | O_CREAT | O_TRUNC
	// a+ or ab+ or a+b = O_RDWR | O_CREAT | O_APPEND

  switch(mode[0]) 
  {
  case 'r':
	  if (mode[1] == '\0')            // r
	  {
		  stream->flag = O_RDONLY;
	  }  
	  else if ( mode[1] == 'b' ) 
	  {    
		  if (mode[2] == '\0')          // rb
		  {
			  stream->flag = O_RDONLY;
		  } 
		  else if (mode[2] == '+')      // rb+
		  {
			  stream->flag = O_RDWR;
		  }			  
	  }
	  else if (mode[1] == '+')        // r+  r+b
	  {
		  stream->flag = O_RDWR;
	  }  
	  break;
  case 'w':
	  if (mode[1] == '\0')            // w
	  {
		  stream->flag = O_WRONLY | O_CREAT | O_TRUNC;
	  }	  
	  else if (mode[1] == 'b') 
	  {
		  if (mode[2] == '\0')          // wb
		  {
			  stream->flag = O_WRONLY | O_CREAT | O_TRUNC;
		  }	  
		  else if (mode[2] == '+')      // wb+
		  {
			  stream->flag = O_RDWR | O_CREAT | O_TRUNC;
		  }	  
	  }
	  else if (mode[1] == '+')        // w+  w+b
	  {
		  stream->flag = O_RDWR | O_CREAT | O_TRUNC;
	  }
	  break;
  case 'a':
	  if (mode[1] == '\0')            // a
	  {
		  stream->flag = O_WRONLY | O_CREAT | O_APPEND;
	  } 
	  else if (mode[1] == 'b')
	  {
		  if (mode[2] == '\0')          // ab
		  {
			  stream->flag = O_WRONLY | O_CREAT | O_APPEND;
		  }  
		  else if (mode[2] == '+')      // ab+
		  {
			  stream->flag = O_RDWR | O_CREAT | O_APPEND;
		  }	  
	  }
	  else if (mode[1] == '+')        // a+  a+b
	  {
		  stream->flag = O_RDWR | O_CREAT | O_APPEND;
	  } 
	  break;
  }
  
  mode_t open_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

  if ((stream->fd = open(path, stream->flag, open_mode)) == -1) 
  {
	  delete stream;
	  printf("fopen failed\n");
	  stream = NULL;
  }
  
  return stream;
}


/*
clears an input/output stream buffer
resets pos and actual_size
*/
int fpurge(FILE *stream)
{
	//fpurge makes sure that the stream and file are in synch with the call to lseek
	//if the file pointer moves ahead of the pos in stream, lseek adjusts by going by actual_size from current pointer location
	lseek(stream->fd, (stream->actual_size * -1), SEEK_CUR);
	*stream->buffer = '\0';
	stream->pos = 0;
	stream->actual_size = 0;
	
	return 0;
}

/*
if buffer is being used for input or nothing was written to the buffer, fflush just calls fpurge
if buffer was written to, a last write call is used to flush the buffer/stream to the file and then fpurge is called
*/
int fflush(FILE *stream)
{
	
	if(stream->flag == O_RDONLY)
 	{
		fpurge(stream);

	} 
	else if (stream->lastop == 'r')
	{
		fpurge(stream);
	}
	else {
		if(stream->pos != 0) 
		{
			stream->lastop = 'w';
			write(stream->fd, stream->buffer, stream->pos);
		}
		fpurge(stream);
	}
	
	return 0;

}

/*
reads a character (one byte) one at a time from a file stream 
if successful returns character read as an int
if at end of file, fgetc returns EOF
*/
int fgetc(FILE *stream)
{

	if(stream->lastop == 'w') {
		fpurge(stream);
	}

	stream->lastop = 'r';

	int actualSize;
	char character;
	if(stream->actual_size == 0)
 	{	
		fpurge(stream);
		actualSize = read(stream->fd, stream->buffer, stream->size);
		stream->actual_size = actualSize;

		//if read returns 0, at end of file
		if(actualSize == 0) {
			stream->eof = true;
			return EOF;
		}

		stream->pos = 0;
		character = stream->buffer[stream->pos];
		stream->actual_size--;
		stream->pos++;
		return character;

	} else
	{
		character = stream->buffer[stream->pos];
		stream->pos++;
		stream->actual_size--;
		return character;
	}

}

/*
reads from a buffer stream or file
if successful returns number of elements successfully read
if number of elements read is less than nmemb, an error occurred or eof was reached
*/
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{

	if(stream->mode == _IONBF) {
		int readNum = read(stream->fd, ptr, (size * nmemb));

			if (readNum == -1) 
			{
				return 0;

			} else
			{
				return readNum / size;
			}
	}

	if(stream != NULL)
	{
		//if #of bytes to be read is greater than buffer size and buffer is empty
		//just do a read system call
		if((size * nmemb) > stream->size && stream->actual_size == 0) {
			int actualRead = read(stream->fd, ptr, (size * nmemb));
			if(actualRead == 0) {
				stream->eof = true;
			}

			if (actualRead == -1) 
			{
				return 0;

			} else
			{
				return actualRead / size;
			}
		}
		else 
		{
			size_t count = 0;
			int character;
			unsigned char *buffPtr = static_cast<unsigned char *>(ptr);

			while(stream->eof != true && count < (nmemb * size)) 
			{
				//loops through and uses fgetc to grab byte of data from stream
				character = fgetc(stream);
				if(stream->eof != true) 
				{
					*buffPtr = character;
					count++;
					*buffPtr++;
				}
		
			}
	
			return count / size;
		}
	}
	else 
	{
		return 0;
	}
}

/*
writes a character to a file stream
if successful, the character written to the stream is returned
if not successful, EOF is returned
*/
int fputc(int c, FILE *stream)
{
	
	if(stream->flag == O_RDONLY)
	{
		return EOF;
	}

	if(stream->lastop == 'r') 
	{
		fpurge(stream);
	}

	stream->lastop = 'w';

	//if end of buffer/stream is reached, flush stream
	if(stream->actual_size == 0 && stream->pos != 0) 
	{
		fflush(stream);
		stream->actual_size = stream->size;

	}
	else if(stream->actual_size == 0 && stream->pos == 0) //if buffer/stream is empty
	{
		stream->actual_size = stream->size;
	}

	stream->buffer[stream->pos] = c;
	stream->pos++;
	stream->actual_size--;
	
	return c;

}

/*
writes to a buffer stream or file
if successful returns, number of elements successfully written
if number of elements written is less than nmemb, an error occurred or eof was reached
*/
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{	

	if(stream->mode == _IONBF) {
		int writeNum = write(stream->fd, ptr, (size * nmemb));

		if (writeNum == -1) 
		{
			return 0;

		} else
		{
			return writeNum / size;
		}
	}

	if((size * nmemb) > stream->size && stream->actual_size == 0)
	{
		//if #of bytes to be written is greater than buffer size and buffer is empty
		//just do a write system call
		int actualWrite = write(stream->fd, ptr, (size * nmemb));

		if (actualWrite == -1) 
		{
			return 0;

		} else
		{
			return actualWrite / size;
		}
	}
	else
	{
		size_t count = 0;
		int character;
		int check;
		const unsigned char *buffPtr = static_cast<const unsigned char *>(ptr);

		while(check != EOF && count < (nmemb * size)) {
			character = *buffPtr;
			check = fputc(character, stream);
			if(check != EOF)
			{
				count++;
				*buffPtr++;
			}
		}
	
		return count / size;
	}
	
}


/*
reads a character string from a file stream
stops either because size is reached, EOF is reached, or '\n' is reached
returns str, unless stream is at eof and nothing was read in which case NULL is returned
*/
char *fgets(char *str, int size, FILE *stream)
{
	char temp = fgetc(stream);

	if(temp == EOF) 
	{
		stream->eof = true;
		return NULL;

	} else 
	{
		str[0] = temp;
		for(int i = 1; i < (size - 1); i++) 
		{
			temp = fgetc(stream);

			if(temp != EOF && temp != '\n') 
			{
				str[i] = temp;
			} 
			else if( temp == '\n')
			{
				str[i] = temp;
				str[i + 1] = '\0'; //null byte written at the end
				return str;	
			}
			else 
			{
				str[i + 1] = '\0'; //null byte written at the end
				return str;
			}
		}

		str[size - 1] = '\0'; //null byte written at the end
		return str;

	}

}

/*
writes a character string to a file stream
stops when it reaches null byte, does not write it to stream
if successful returns non-negative number (0)
if not successful EOF is returned
*/
int fputs(const char *str, FILE *stream)
{	
	if(stream->flag == O_RDONLY)
	{
		return EOF;
	}

	int count = 0;
	int putReturn;

	while(str[count] != '\0') 
	{
		putReturn = fputc(str[count], stream);
		count++;
	}
	
	return 0;
}

int feof(FILE *stream) 
{
	return stream->eof == true;
}

/*
moves the file position to a specific location in a file 
adjusts location of pointer in file and pos in stream based on offset and whence
resets eof flag to false
*/
int fseek(FILE *stream, long offset, int whence)
{

	fflush(stream);
	lseek(stream->fd, offset, whence);

	stream->eof = false;

	if(whence == SEEK_CUR) 
	{
		stream->pos += offset;
	} 
	else if(whence == SEEK_SET) 
	{
		stream->pos = offset;
	} 
	else if(whence == SEEK_END)
	{
		stream->pos = stream->size - offset;
	}
	else 
	{
		return -1;
	}

	return 0;
}

/*
closes a file
flushes stream (writes anything from output buffer, or just purges input buffer)
close file descriptor
if buffer is owend by stdio, memory is deallocated
*/
int fclose(FILE *stream)
{
	fflush(stream);
	close(stream->fd);

	if(stream->bufown == true) 
	{
		delete stream;
		stream = nullptr;
	}
	return 0;
}
