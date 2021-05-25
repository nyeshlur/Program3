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

int fpurge(FILE *stream)
{
	lseek(stream->fd, (stream->actual_size * -1), SEEK_CUR);
	*stream->buffer = '\0';
	stream->pos = 0;
	stream->actual_size = 0;
	
	return 0;

	//if error occurs return -1, when would an error occur?
}

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

	//if error occurs return EOF, when would an error occur?
}

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

		if(actualSize == 0) {
			stream->eof = true;
			return EOF; //what other cases should return EOF?
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

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	if(stream != NULL)
	{

		if((size * nmemb) > stream->size && stream->actual_size == 0) {
			int actualRead = read(stream->fd, ptr, (size * nmemb));
			if(actualRead == 0) {
				stream->eof = true;
			}
			return actualRead / size;
		} 
		else 
		{
			size_t count = 0;
			int character;
			unsigned char *buffPtr = static_cast<unsigned char *>(ptr);

			while(stream->eof != true && count < (nmemb * size)) {
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

int fputc(int c, FILE *stream) // complete it
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

	if(stream->actual_size == 0 && stream->pos != 0) 
	{
		fflush(stream);
		stream->actual_size = stream->size;

	} 
	else if(stream->actual_size == 0 && stream->pos == 0)
	{
		stream->actual_size = stream->size;
	}

	stream->buffer[stream->pos] = c;
	stream->pos++;
	stream->actual_size--;
	
	return c;

}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) // complete it
{	
	if((size * nmemb) > stream->size && stream->actual_size == 0)
	{
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
		for(int i = 1; i < size; i++) 
		{
			temp = fgetc(stream);

			if(temp != EOF) 
			{
				str[i] = temp;
			} else 
			{
				return str;
			}
		}

		return str;

	}

}

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

int fseek(FILE *stream, long offset, int whence)
{

	fflush(stream);
	lseek(stream->fd, offset, whence);

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
