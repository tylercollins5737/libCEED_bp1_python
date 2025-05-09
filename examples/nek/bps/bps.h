// Copyright (c) 2017-2025, Lawrence Livermore National Security, LLC and other CEED contributors.
// All Rights Reserved. See the top-level LICENSE and NOTICE files for details.
//
// SPDX-License-Identifier: BSD-2-Clause
//
// This file is part of CEED:  http://github.com/ceed
#pragma once

#include <ceed/types.h>
#ifndef CEED_RUNNING_JIT_PASS
#include <math.h>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// *****************************************************************************
//   BP 1
// *****************************************************************************
CEED_QFUNCTION(masssetupf)(void *ctx, CeedInt Q, const CeedScalar *const *in, CeedScalar *const *out) {
  CeedScalar       *qdata = out[0], *rhs = out[1];
  const CeedScalar *x = in[0];
  const CeedScalar *J = in[1];
  const CeedScalar *w = in[2];

  // Quadrature Point Loop
  for (CeedInt i = 0; i < Q; i++) {
    CeedScalar det = (J[i + Q * 0] * (J[i + Q * 4] * J[i + Q * 8] - J[i + Q * 5] * J[i + Q * 7]) -
                      J[i + Q * 1] * (J[i + Q * 3] * J[i + Q * 8] - J[i + Q * 5] * J[i + Q * 6]) +
                      J[i + Q * 2] * (J[i + Q * 3] * J[i + Q * 7] - J[i + Q * 4] * J[i + Q * 6]));
    qdata[i]       = det * w[i];
    rhs[i]         = qdata[i] * sqrt(x[i] * x[i] + x[i + Q] * x[i + Q] + x[i + 2 * Q] * x[i + 2 * Q]);
  }  // End of Quadrature Point Loop
  return 0;
}

CEED_QFUNCTION(massf)(void *ctx, CeedInt Q, const CeedScalar *const *in, CeedScalar *const *out) {
  const CeedScalar *u     = in[0];
  const CeedScalar *qdata = in[1];
  CeedScalar       *v     = out[0];

  // Quadrature Point Loop
  for (CeedInt i = 0; i < Q; i++) v[i] = qdata[i] * u[i];

  return 0;
}
// *****************************************************************************
//   BP 3
// *****************************************************************************
CEED_QFUNCTION(diffsetupf)(void *ctx, CeedInt Q, const CeedScalar *const *in, CeedScalar *const *out) {
  const CeedScalar *x     = in[0];
  const CeedScalar *J     = in[1];
  const CeedScalar *w     = in[2];
  CeedScalar       *qdata = out[0], *rhs = out[1];

  // Quadrature Point Loop
  for (CeedInt i = 0; i < Q; i++) {
    // Stored in Voigt convention
    // 0 5 4
    // 5 1 3
    // 4 3 2
    const CeedScalar J11  = J[i + Q * 0];
    const CeedScalar J21  = J[i + Q * 1];
    const CeedScalar J31  = J[i + Q * 2];
    const CeedScalar J12  = J[i + Q * 3];
    const CeedScalar J22  = J[i + Q * 4];
    const CeedScalar J32  = J[i + Q * 5];
    const CeedScalar J13  = J[i + Q * 6];
    const CeedScalar J23  = J[i + Q * 7];
    const CeedScalar J33  = J[i + Q * 8];
    const CeedScalar A11  = J22 * J33 - J23 * J32;
    const CeedScalar A12  = J13 * J32 - J12 * J33;
    const CeedScalar A13  = J12 * J23 - J13 * J22;
    const CeedScalar A21  = J23 * J31 - J21 * J33;
    const CeedScalar A22  = J11 * J33 - J13 * J31;
    const CeedScalar A23  = J13 * J21 - J11 * J23;
    const CeedScalar A31  = J21 * J32 - J22 * J31;
    const CeedScalar A32  = J12 * J31 - J11 * J32;
    const CeedScalar A33  = J11 * J22 - J12 * J21;
    const CeedScalar qw   = w[i] / (J11 * A11 + J21 * A12 + J31 * A13);
    qdata[i + Q * 0]      = qw * (A11 * A11 + A12 * A12 + A13 * A13);
    qdata[i + Q * 1]      = qw * (A21 * A21 + A22 * A22 + A23 * A23);
    qdata[i + Q * 2]      = qw * (A31 * A31 + A32 * A32 + A33 * A33);
    qdata[i + Q * 3]      = qw * (A21 * A31 + A22 * A32 + A23 * A33);
    qdata[i + Q * 4]      = qw * (A11 * A31 + A12 * A32 + A13 * A33);
    qdata[i + Q * 5]      = qw * (A11 * A21 + A12 * A22 + A13 * A23);
    const CeedScalar c[3] = {0, 1., 2.};
    const CeedScalar k[3] = {1., 2., 3.};
    const CeedScalar rho  = w[i] * (J11 * A11 + J21 * A12 + J31 * A13);
    rhs[i]                = rho * M_PI * M_PI * (k[0] * k[0] + k[1] * k[1] + k[2] * k[2]) * sin(M_PI * (c[0] + k[0] * x[i + Q * 0])) *
             sin(M_PI * (c[1] + k[1] * x[i + Q * 1])) * sin(M_PI * (c[2] + k[2] * x[i + Q * 2]));
  }  // End of Quadrature Point Loop
  return 0;
}

CEED_QFUNCTION(diffusionf)(void *ctx, CeedInt Q, const CeedScalar *const *in, CeedScalar *const *out) {
  const CeedScalar *ug    = in[0];
  const CeedScalar *qdata = in[1];
  CeedScalar       *vg    = out[0];

  // Quadrature Point Loop
  for (CeedInt i = 0; i < Q; i++) {
    const CeedScalar ug0 = ug[i + Q * 0];
    const CeedScalar ug1 = ug[i + Q * 1];
    const CeedScalar ug2 = ug[i + Q * 2];
    vg[i + Q * 0]        = qdata[i + Q * 0] * ug0 + qdata[i + Q * 5] * ug1 + qdata[i + Q * 4] * ug2;
    vg[i + Q * 1]        = qdata[i + Q * 5] * ug0 + qdata[i + Q * 1] * ug1 + qdata[i + Q * 3] * ug2;
    vg[i + Q * 2]        = qdata[i + Q * 4] * ug0 + qdata[i + Q * 3] * ug1 + qdata[i + Q * 2] * ug2;
  }  // End of Quadrature Point Loop
  return 0;
}
