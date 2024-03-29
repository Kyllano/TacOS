/* hello.c
 *	Simple hello world program
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

#include "userlib/libnachos.h"
#include "userlib/syscall.h"

int
main() {

  SemId sid = SemCreate("coucou", 1);
  P(sid);
  n_printf("** ** ** PROUT  ** ** **\n");
  n_printf("** ** ** Bonjour le monde ** ** **\n");
  SemDestroy(sid);
  t_error err = V(sid);
  n_printf("coucou : %d\n", err);
  PError("coucou");
  n_printf("** ** ** Bonjour AGAIN ** ** **\n");

  return 0;
}
