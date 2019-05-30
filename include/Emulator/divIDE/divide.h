/* 
 * Copyright (C) 2005-2009 Dusan Gallo
 * Email: dusky@hq.alert.sk
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. See the file COPYING. 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Rev: 12 $
 *
 */

#ifndef DIVIDE_H
#define DIVIDE_H

#define DIVIDE_VERSION "v0.3 ($Rev: 12 $)"

void divide_init();
void divide_exit();
void divide_attach_drive(int drive, char *path);
void divide_detach_drive(int drive);

void divide_premap(int addr);
void divide_postmap(int addr);
void divide_mapper();
void divide_put_mem(int addr, byte *ptr, byte val);

void divide_port_out(int portl, byte data);
int divide_port_in(int portl);

void divide_select_drive(int drive);
void divide_switch_jp2();

#endif /* DIVIDE_H */
