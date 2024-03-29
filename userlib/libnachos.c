/*! \file libnachos.c
 *  \brief Functions of our library, for user programs.
 *
 * This library only provides some  usefull functions for
 * programming.
 *
 * -----------------------------------------------------
 * This file is part of the Nachos-RiscV distribution
 * Copyright (c) 2022 University of Rennes 1.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details
 * (see see <http://www.gnu.org/licenses/>).
 * -----------------------------------------------------
 */

#include "libnachos.h"
#include <stdarg.h>
#include <stdint.h>

//----------------------------------------------------------------------
// threadStart()
/*!	Makes a thread execute a function or program. This function
//      is static, it is called internally by library function threadCreate
//      and should not be called directly by user programs. The interest
//      of this function is to be able to terminate threads correctly,
//      even when the thread to be terminated does not explicitly call
//      Exit. threadStart provides the mechanism by which Exit
//      is called automatically
//
//	\param func is the identificator of the function to execute.
*/
//----------------------------------------------------------------------

static void
threadStart(uint64_t func) {
  VoidNoArgFunctionPtr func2;
  func2 = (VoidNoArgFunctionPtr) func;
  // Call the function that actually contains the thread code
  (*func2)();
  // Call exit, such that there is no return using an empty stack
  Exit(0);
}

//----------------------------------------------------------------------
// threadCreate()
/*!	 Creates a thread and makes it execute a function.
//
//      NB : instead of directly executing the required function,
//           function threadStart is called so as to ensure
//           that the thread will properly exit
//      This function must be called instead of calling directly
//      the system call newThread
//
//      \param name the name of the thread (for debugging purpose)
//	\param func is the address of the function to execute.
*/
//----------------------------------------------------------------------
ThreadId
threadCreate(char *debug_name, VoidNoArgFunctionPtr func) {
  return newThread(debug_name, (uint64_t) threadStart, (uint64_t) func);
}

//----------------------------------------------------------------------
// n_strcmp()
/*!	String comparison
//
//	\param s1 is the first string,
//	\param s2 is the second one.
//	\return an integer greater than, equal to, or less than 0,
//	  if the first string is greater than, equal to, or less than
//	  the the second string.
*/
//----------------------------------------------------------------------
int
n_strcmp(const char *s1, const char *s2) {
  int comparaison;
  int fini = 0;
  int i = 0;
  while (!fini) {
    if ((s1[i] == 0) && (s2[i] == 0)) {
      fini = 1;
      comparaison = 0;
    }
    if (s1[i] < s2[i]) {
      fini = 1;
      comparaison = -1;
    }
    if (s1[i] > s2[i]) {
      fini = 1;
      comparaison = 1;
    }
    i++;
  }
  return comparaison;
}

//----------------------------------------------------------------------
// n_strcpy()
/*!	String copy
//
//	\param dst is where the string is to be copied,
//	\param src is where the string to copy is.
//	\return dst, if the copy successes, 0 otherwise
*/
//----------------------------------------------------------------------
char *
n_strcpy(char *dst, const char *src) {
  int i = 0;
  int fini = 0;
  if ((dst != 0) && (src != 0)) {
    while (fini == 0) {
      if (src[i] == '\0')
        fini = 1;
      dst[i] = src[i];
      i++;
    }
    return dst;
  } else
    return 0;
}

//----------------------------------------------------------------------
// n_strlen()
/*!	Gives the number of bytes in a string, not including the
//	terminating null character.
//
//	\param c is a pointer onto a string.
//	\return the length of the string.
*/
//----------------------------------------------------------------------
size_t
n_strlen(const char *s) {
  size_t i = 0;
  while (s[i] != 0)
    i++;
  return i;
}

//----------------------------------------------------------------------
// n_strcat()
/*!	Appends a copy of a string, including null character, to the end
//	of another string. Enough memory has to be available in the
//      destination string.
//
//	\param dst is a pointer onto the string where the other string
//	will be appended.
//	\param src is the string to append.
//	\return the pointer string dst.
*/
//----------------------------------------------------------------------
char *
n_strcat(char *dst, const char *src) {
  int i, j, k;
  i = (int) n_strlen(dst);
  j = (int) n_strlen(src);
  for (k = i; k <= j + i; k++) {
    dst[k] = src[k - i];
  }
  return dst;
}

//----------------------------------------------------------------------
// n_toupper()
/*!	Gives the upper-case letter corresponding to the lower-case
//	letter passed as parameter.
//
//	\param c is the ASCII code of the letter to transform.
//	\return the corresponding upper-case letter
*/
//----------------------------------------------------------------------
int
n_toupper(int c) {
  if ((c >= 'a') && (c <= 'z'))
    return c + ('A' - 'a');
  else
    return c;
}

//----------------------------------------------------------------------
// n_tolower()
/*!	Gives the lower-case letter corresponding to the upper-case
//	letter passed as parameter
//
//	\param c is the ASCII code of the letter to transform.
//	\return the corresponding lower-case letter
*/
//----------------------------------------------------------------------
int
n_tolower(int c) {
  if ((c <= 'Z') && (c >= 'A'))
    return c + ('a' - 'A');
  else
    return c;
}

//----------------------------------------------------------------------
// n_atoi()
/*!	String to integer conversion.
//
//	\param c is a pointer onto a string.
//	\return the corresponding value
*/
//----------------------------------------------------------------------
int
n_atoi(const char *str) {
  int i = 0;
  int fini = 0;
  int val = 0;
  int negative = 0;
  if (str[i] == '-') {
    negative = 1;
    i = 1;
  }
  while (!fini) {
    if (str[i] == 0 || str[i] < '0' || str[i] > '9')
      fini = 1;
    else {
      val *= 10;
      val += str[i] - '0';
      i++;
    }
  }
  if (negative)
    return (-val);
  else
    return val;
}

//----------------------------------------------------------------------
// n_memcmp()
/*!	Memory comparison.
//
//	\param s1 is the first memory area,
//	\param s2 is the second memory area.
//      \param n size in bytes of the area to be compared.
//	\return an integer less than, equal to, or greater than 0,
//	according as s1 is lexicographically less than, equal to,
//	or greater than s2 when taken to be unsigned characters.
//
*/
//----------------------------------------------------------------------
int
n_memcmp(const void *s1, const void *s2, size_t n) {
  unsigned char *c1 = (unsigned char *) s1;
  unsigned char *c2 = (unsigned char *) s2;

  int comparaison = 0;
  int fini = 0;
  int i = 0;
  while ((!fini) && (i < n)) {
    if (c1[i] < c2[i]) {
      fini = 1;
      comparaison = -1;
    }
    if (c1[i] > c2[i]) {
      fini = 1;
      comparaison = 1;
    }
    i++;
  }
  return comparaison;
}

//----------------------------------------------------------------------
// n_memcpy()
/*!	Memory copy.
//
//	\param s1 is where the elements are to be copied,
//	\param s2 is the memory area to copy.
//      \param n size in bytes of the area to be copied.
//	\return the memory area where the copy has been done.
*/
//----------------------------------------------------------------------
void *
n_memcpy(void *s1, const void *s2, size_t n) {

  unsigned char *c1 = (unsigned char *) s1;
  unsigned char *c2 = (unsigned char *) s2;

  int i = 0;
  if ((c1 != 0) && (c2 != 0)) {
    while (i < n) {
      c1[i] = c2[i];
      i++;
    }
    return (void *) c1;
  } else
    return 0;
}

//----------------------------------------------------------------------
// n_memset()
/*!	Sets the first n bytes of a memory area to a value (converted to
//	an unsigned char).
//
//	\param s is the memory area to transform,
//	\param c is the value wanted,
//	\param n is the number of bytes to put at c.
//	\return s.
*/
//----------------------------------------------------------------------
void *
n_memset(void *s, int c, size_t n) {
  unsigned char *c1 = (unsigned char *) s;
  int i;
  for (i = 0; i < n; i++) {
    c1[i] = c;
  }
  return (void *) c1;
}

//----------------------------------------------------------------------
// n_dumpmem()
/*!	Dumps on the string the n first bytes of a memory area
//      (used for debugging)
//
//	\param addr address of the memory area
//	\param len number of bytes to be dumped
*/
//----------------------------------------------------------------------
void
n_dumpmem(char *addr, int len) {
#define TOHEX(x)                                                               \
  ({                                                                           \
    char __x = (x);                                                            \
    if (__x < 10)                                                              \
      __x += '0';                                                              \
    else                                                                       \
      __x = 'a' + (__x - 10);                                                  \
    __x;                                                                       \
  })

  int i;
  for (i = 0; i < len; i++) {
    char s[3];
    if ((i % 16) == 0)
      n_printf("%x\t", (unsigned long) &addr[i]);
    else if ((i % 8) == 0)
      n_printf("   ");
    s[0] = TOHEX((addr[i] >> 4) & 0xf);
    s[1] = TOHEX(addr[i] & 0xf);
    s[2] = '\0';
    n_printf("%s ", s);
    if ((((i + 1) % 16) == 0) || (i == len - 1))
      n_printf("\n");
  }
}

#define PUTCHAR(carac)                                                         \
  do {                                                                         \
    if (result < len - 1)                                                      \
      *buff++ = carac;                                                         \
    result++;                                                                  \
  } while (0)

//----------------------------------------------------------------------
// n_vsnprintf()
/*!	Build a string according to a specified format (internal function)
//
//	Nachos vsnprintf accepts:
//		%c to print a character,
//		%s, to print a string,
//		%d, to print an integer,
//		%x, to print an integer in hexa
//              %lx %ld same for 64-bit values
//              %f, to print a floating point value
//
//      \param buff the destination buffer to generate the string to
//      \param len the size of buff, determines the number max of
//        characters copied to buff (taking the final \0 into account)
//	\param format the string to parse
//	\param ap parameters to print
//
//      \return the number of characters formatted (NOT including \0),
//        that is, the number of characters that would have been written
//        to the buffer if it were large enough. -1 on error.
*/
//----------------------------------------------------------------------
static int
n_vsnprintf(char *buff, int len, const char *format, va_list ap) {
  int i, result;

  if (!buff || !format || (len < 0)) {
    return -1;
  }
  result = 0;

  for (i = 0; format[i] != '\0'; i++) {
    switch (format[i]) {
    case '%':
      i++;
      switch (format[i]) {
      case '%': {
        PUTCHAR('%');
        break;
      }
      case 'i':
      case 'd': {
        int integer = (int) va_arg(ap, int);
        int cpt2 = 0;
        char buff_int[11];
        if (integer < 0) {
          PUTCHAR('-');
        }
        do {
          int m10 = integer % 10;
          m10 = (m10 < 0) ? -m10 : m10;
          buff_int[cpt2++] = (char) ('0' + m10);
          integer = integer / 10;
        } while (integer != 0);
        for (cpt2 = cpt2 - 1; cpt2 >= 0; cpt2--) {
          PUTCHAR(buff_int[cpt2]);
        }
        break;
      }
      case 'l': {
        i++;
        switch (format[i]) {
        case 'd': {
          long longer = va_arg(ap, long);
          int cpt2 = 0;
          char buff_long[20];
          if (longer < 0) {
            PUTCHAR('-');
          }
          do {
            int m10 = longer % 10;
            m10 = (m10 < 0) ? -m10 : m10;
            buff_long[cpt2++] = (char) ('0' + m10);
            longer = longer / 10;
          } while (longer != 0);
          for (cpt2 = cpt2 - 1; cpt2 >= 0; cpt2--) {
            PUTCHAR(buff_long[cpt2]);
          }
          break;
        }
        case 'x': {
          uint64_t hexa = va_arg(ap, long);
          uint64_t nb;
          uint32_t i, had_nonzero = 0;
          for (i = 0; i < 16; i++) {
            nb = (hexa << (i * 4));
            nb = (nb >> 60);
            nb = nb & 0x000000000000000f;
            // Skip the leading zeros
            if (nb == 0) {
              if (had_nonzero) {
                PUTCHAR((uint8_t) '0');
              }
            } else {
              had_nonzero = 1;
              if (nb < 10)
                PUTCHAR((uint8_t) '0' + (uint8_t) nb);
              else
                PUTCHAR((uint8_t) 'a' + (uint8_t) (nb - 10));
            }
          }
          if (!had_nonzero)
            PUTCHAR((uint8_t) '0');
          break;
        }
        default: {
          PUTCHAR('%');
          PUTCHAR('l');
          PUTCHAR(format[i]);
          break;
        }
        }

        break;
      }
      case 'c': {
        int value = va_arg(ap, int);
        PUTCHAR((char) value);
        break;
      }
      case 's': {
        char *string = va_arg(ap, char *);
        if (!string)
          string = "(null)";
        for (; *string != '\0'; string++)
          PUTCHAR(*string);
        break;
      }
      case 'x': {
        uint32_t hexa = va_arg(ap, int);
        uint32_t nb;
        uint32_t i, had_nonzero = 0;
        for (i = 0; i < 8; i++) {
          nb = (hexa << (i * 4));
          nb = (nb >> 28);
          nb = nb & 0x0000000f;
          // Skip the leading zeros
          if (nb == 0) {
            if (had_nonzero)
              PUTCHAR((uint8_t) '0');
          } else {
            had_nonzero = 1;
            if (nb < 10)
              PUTCHAR((uint8_t) '0' + (uint8_t) nb);
            else
              PUTCHAR((uint8_t) 'a' + (uint8_t) (nb - 10));
          }
        }
        if (!had_nonzero)
          PUTCHAR((uint8_t) '0');
        break;
      }
        /*case 'f': {
        // Very simple routine to print floats as xxxx.yyyyy
        // Not very good (unable to print large numbers)
        // If anyone wants to re-write it, feel free ...
        double f = (double) va_arg(ap,double);
        int cpt2, j;
        char buff_float[200];
        long ient,idec;
        if (f<0) {
          PUTCHAR('-');
          f = -f;
        }
        ient = (int)f;
        // 100000 = print 5 digits max
        idec = (int)((f - ((double)ient))*100000);
        // Round up
        if ( f - ((double)ient) - ((double)idec)/100000.0 >= 0.5E-5)
          idec ++;
        cpt2 = 0;
        // Print digits after the '.'
        for (j=0 ; j<5 ; j++) {
          buff_float[cpt2++]=(char)('0'+(idec%10));
          idec=idec/10;
        }
        buff_float[cpt2++] = '.';
        // Print digits before the '.'
        do {
          buff_float[cpt2++]=(char)('0'+ (ient%10));
          ient=ient/10;
        } while (ient!=0);
        for(j = cpt2 - 1 ; j >= 0 ; j--)
          PUTCHAR(buff_float[j]);
        break;
      }
        */
      default:
        PUTCHAR('%');
        PUTCHAR(format[i]);
      }
      break;
    default:
      PUTCHAR(format[i]);
    }
  }
  *buff = '\0';
  return result;
}

//----------------------------------------------------------------------
// n_snprintf()
/*!	Build a string according to a specified format
//
//	Nachos snprintf accepts:
//		%c to print a character,
//		%s, to print a string,
//		%d, to print an integer,
//		%x, to print a string in hexa
//              %f, to print a floating point value
//
//      \param buff the destination buffer to generate the string to
//      \param len the size of buff, determines the number max of
//        characters copied to buff (taking the final \0 into account)
//	\param format the string to parse
//	\param ... the (variable number of) arguments
//
//      \return the number of characters formatted (NOT including \0),
//        that is, the number of characters that would have been written
//        to the buffer if it were large enough. -1 on error.
*/
//----------------------------------------------------------------------
int
n_snprintf(char *buff, int len, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  len = n_vsnprintf(buff, len, format, ap);
  va_end(ap);
  return len;
}

//----------------------------------------------------------------------
// n_printf()
/*!	Print to the standard output parameters.
//
//	Nachos printf accepts:
//		%c to print a character,
//		%s, to print a string,
//		%d, to print an integer,
//		%x, to print a string in hexa
//              %ld, %lx, same for 64-bit values
//              %f, to print a floating point value
//
//	\param parameters to print,
//	\param type of print.
*/
//----------------------------------------------------------------------
void
n_printf(const char *format, ...) {

  va_list ap;
  char buff[200];
  int len;

  va_start(ap, format);
  len = n_vsnprintf(buff, sizeof(buff), format, ap);
  va_end(ap);

  if (len >= sizeof(buff)) {
    len = sizeof(buff) - 1;
  }
  if (len > 0) {
    Write(buff, len, CONSOLE_OUTPUT);
  }
}

//----------------------------------------------------------------------
// n_read_int()
/*!
// Very basic minimalist read integer function, no error
// checking...
*/
//----------------------------------------------------------------------
int
n_read_int(void) {
  char buff[200];
  Read(buff, 200, CONSOLE_INPUT);
  return n_atoi(buff);
}
