/*
 * idc.cpp - DC current source class implementation
 *
 * Copyright (C) 2003, 2004 Stefan Jahn <stefan@lkcc.org>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this package; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
 *
 * $Id: idc.cpp,v 1.7 2004-11-24 19:15:48 raimi Exp $
 *
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include "complex.h"
#include "object.h"
#include "node.h"
#include "circuit.h"
#include "component_id.h"
#include "idc.h"

idc::idc () : circuit (2) {
  type = CIR_IDC;
  setISource (true);
}

void idc::initSP (void) {
  allocMatrixS ();
  setS (1, 1, 1.0);
  setS (1, 2, 0.0);
  setS (2, 1, 0.0);
  setS (2, 2, 1.0);
}

void idc::initDC (void) {
  nr_double_t i = getPropertyDouble ("I");
  allocMatrixMNA ();
  setI (1, +i); setI (2, -i);
}

void idc::initAC (void) {
  allocMatrixMNA ();
  clearI ();
}

void idc::initTR (void) {
  initDC ();
}
