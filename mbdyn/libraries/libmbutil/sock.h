/*
 * MBDyn (C) is a multibody analysis code.
 * http://www.mbdyn.org
 *
 * Copyright (C) 1996-2005
 *
 * Pierangelo Masarati	<masarati@aero.polimi.it>
 * Paolo Mantegazza	<mantegazza@aero.polimi.it>
 *
 * Dipartimento di Ingegneria Aerospaziale - Politecnico di Milano
 * via La Masa, 34 - 20156 Milano, Italy
 * http://www.aero.polimi.it
 *
 * Changing this copyright notice is forbidden.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation (version 2 of the License).
 *
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef SOCK_H
#define SOCK_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef HAVE_SOCKLEN_T
typedef int socklen_t;
#endif /* HAVE_SOCKLEN_T */

extern int
mbdyn_make_inet_socket(struct sockaddr_in *name, const char *hostname,
		unsigned short int port, int dobind, int *perror);

extern int
mbdyn_make_named_socket(const char *path, int dobind, int *perror);

#if 0
extern int
make_nonblocking(int sock);

extern int
make_blocking(int sock);
#endif

extern int
mbdyn_connect(int sock, struct sockaddr * addr, socklen_t dim, int _count);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SOCK_H */
