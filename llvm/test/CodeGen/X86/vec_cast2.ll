; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=i386-apple-darwin10 -mattr=+avx | FileCheck %s
; RUN: llc < %s -mtriple=i386-apple-darwin10 -mattr=+avx -x86-experimental-vector-widening-legalization | FileCheck %s --check-prefix=CHECK-WIDE

define <8 x float> @cvt_v8i8_v8f32(<8 x i8> %src) {
; CHECK-LABEL: cvt_v8i8_v8f32:
; CHECK:       ## %bb.0:
; CHECK-NEXT:    vpmovsxbd %xmm0, %xmm1
; CHECK-NEXT:    vpshufd {{.*#+}} xmm0 = xmm0[1,1,2,3]
; CHECK-NEXT:    vpmovsxbd %xmm0, %xmm0
; CHECK-NEXT:    vinsertf128 $1, %xmm0, %ymm1, %ymm0
; CHECK-NEXT:    vcvtdq2ps %ymm0, %ymm0
; CHECK-NEXT:    retl
;
; CHECK-WIDE-LABEL: cvt_v8i8_v8f32:
; CHECK-WIDE:       ## %bb.0:
; CHECK-WIDE-NEXT:    vpmovsxbd %xmm0, %xmm1
; CHECK-WIDE-NEXT:    vpshufd {{.*#+}} xmm0 = xmm0[1,1,2,3]
; CHECK-WIDE-NEXT:    vpmovsxbd %xmm0, %xmm0
; CHECK-WIDE-NEXT:    vinsertf128 $1, %xmm0, %ymm1, %ymm0
; CHECK-WIDE-NEXT:    vcvtdq2ps %ymm0, %ymm0
; CHECK-WIDE-NEXT:    retl
  %res = sitofp <8 x i8> %src to <8 x float>
  ret <8 x float> %res
}

define <8 x float> @cvt_v8i16_v8f32(<8 x i16> %src) {
; CHECK-LABEL: cvt_v8i16_v8f32:
; CHECK:       ## %bb.0:
; CHECK-NEXT:    vpmovsxwd %xmm0, %xmm1
; CHECK-NEXT:    vpshufd {{.*#+}} xmm0 = xmm0[2,3,0,1]
; CHECK-NEXT:    vpmovsxwd %xmm0, %xmm0
; CHECK-NEXT:    vinsertf128 $1, %xmm0, %ymm1, %ymm0
; CHECK-NEXT:    vcvtdq2ps %ymm0, %ymm0
; CHECK-NEXT:    retl
;
; CHECK-WIDE-LABEL: cvt_v8i16_v8f32:
; CHECK-WIDE:       ## %bb.0:
; CHECK-WIDE-NEXT:    vpmovsxwd %xmm0, %xmm1
; CHECK-WIDE-NEXT:    vpshufd {{.*#+}} xmm0 = xmm0[2,3,0,1]
; CHECK-WIDE-NEXT:    vpmovsxwd %xmm0, %xmm0
; CHECK-WIDE-NEXT:    vinsertf128 $1, %xmm0, %ymm1, %ymm0
; CHECK-WIDE-NEXT:    vcvtdq2ps %ymm0, %ymm0
; CHECK-WIDE-NEXT:    retl
  %res = sitofp <8 x i16> %src to <8 x float>
  ret <8 x float> %res
}

define <4 x float> @cvt_v4i8_v4f32(<4 x i8> %src) {
; CHECK-LABEL: cvt_v4i8_v4f32:
; CHECK:       ## %bb.0:
; CHECK-NEXT:    vpmovsxbd %xmm0, %xmm0
; CHECK-NEXT:    vcvtdq2ps %xmm0, %xmm0
; CHECK-NEXT:    retl
;
; CHECK-WIDE-LABEL: cvt_v4i8_v4f32:
; CHECK-WIDE:       ## %bb.0:
; CHECK-WIDE-NEXT:    vpmovsxbd %xmm0, %xmm0
; CHECK-WIDE-NEXT:    vcvtdq2ps %xmm0, %xmm0
; CHECK-WIDE-NEXT:    retl
  %res = sitofp <4 x i8> %src to <4 x float>
  ret <4 x float> %res
}

define <4 x float> @cvt_v4i16_v4f32(<4 x i16> %src) {
; CHECK-LABEL: cvt_v4i16_v4f32:
; CHECK:       ## %bb.0:
; CHECK-NEXT:    vpmovsxwd %xmm0, %xmm0
; CHECK-NEXT:    vcvtdq2ps %xmm0, %xmm0
; CHECK-NEXT:    retl
;
; CHECK-WIDE-LABEL: cvt_v4i16_v4f32:
; CHECK-WIDE:       ## %bb.0:
; CHECK-WIDE-NEXT:    vpmovsxwd %xmm0, %xmm0
; CHECK-WIDE-NEXT:    vcvtdq2ps %xmm0, %xmm0
; CHECK-WIDE-NEXT:    retl
  %res = sitofp <4 x i16> %src to <4 x float>
  ret <4 x float> %res
}

define <8 x float> @cvt_v8u8_v8f32(<8 x i8> %src) {
; CHECK-LABEL: cvt_v8u8_v8f32:
; CHECK:       ## %bb.0:
; CHECK-NEXT:    vpmovzxbd {{.*#+}} xmm1 = xmm0[0],zero,zero,zero,xmm0[1],zero,zero,zero,xmm0[2],zero,zero,zero,xmm0[3],zero,zero,zero
; CHECK-NEXT:    vpshufd {{.*#+}} xmm0 = xmm0[1,1,2,3]
; CHECK-NEXT:    vpmovzxbd {{.*#+}} xmm0 = xmm0[0],zero,zero,zero,xmm0[1],zero,zero,zero,xmm0[2],zero,zero,zero,xmm0[3],zero,zero,zero
; CHECK-NEXT:    vinsertf128 $1, %xmm0, %ymm1, %ymm0
; CHECK-NEXT:    vcvtdq2ps %ymm0, %ymm0
; CHECK-NEXT:    retl
;
; CHECK-WIDE-LABEL: cvt_v8u8_v8f32:
; CHECK-WIDE:       ## %bb.0:
; CHECK-WIDE-NEXT:    vpmovzxbd {{.*#+}} xmm1 = xmm0[0],zero,zero,zero,xmm0[1],zero,zero,zero,xmm0[2],zero,zero,zero,xmm0[3],zero,zero,zero
; CHECK-WIDE-NEXT:    vpshufd {{.*#+}} xmm0 = xmm0[1,1,2,3]
; CHECK-WIDE-NEXT:    vpmovzxbd {{.*#+}} xmm0 = xmm0[0],zero,zero,zero,xmm0[1],zero,zero,zero,xmm0[2],zero,zero,zero,xmm0[3],zero,zero,zero
; CHECK-WIDE-NEXT:    vinsertf128 $1, %xmm0, %ymm1, %ymm0
; CHECK-WIDE-NEXT:    vcvtdq2ps %ymm0, %ymm0
; CHECK-WIDE-NEXT:    retl
  %res = uitofp <8 x i8> %src to <8 x float>
  ret <8 x float> %res
}

define <8 x float> @cvt_v8u16_v8f32(<8 x i16> %src) {
; CHECK-LABEL: cvt_v8u16_v8f32:
; CHECK:       ## %bb.0:
; CHECK-NEXT:    vpxor %xmm1, %xmm1, %xmm1
; CHECK-NEXT:    vpunpckhwd {{.*#+}} xmm1 = xmm0[4],xmm1[4],xmm0[5],xmm1[5],xmm0[6],xmm1[6],xmm0[7],xmm1[7]
; CHECK-NEXT:    vpmovzxwd {{.*#+}} xmm0 = xmm0[0],zero,xmm0[1],zero,xmm0[2],zero,xmm0[3],zero
; CHECK-NEXT:    vinsertf128 $1, %xmm1, %ymm0, %ymm0
; CHECK-NEXT:    vcvtdq2ps %ymm0, %ymm0
; CHECK-NEXT:    retl
;
; CHECK-WIDE-LABEL: cvt_v8u16_v8f32:
; CHECK-WIDE:       ## %bb.0:
; CHECK-WIDE-NEXT:    vpxor %xmm1, %xmm1, %xmm1
; CHECK-WIDE-NEXT:    vpunpckhwd {{.*#+}} xmm1 = xmm0[4],xmm1[4],xmm0[5],xmm1[5],xmm0[6],xmm1[6],xmm0[7],xmm1[7]
; CHECK-WIDE-NEXT:    vpmovzxwd {{.*#+}} xmm0 = xmm0[0],zero,xmm0[1],zero,xmm0[2],zero,xmm0[3],zero
; CHECK-WIDE-NEXT:    vinsertf128 $1, %xmm1, %ymm0, %ymm0
; CHECK-WIDE-NEXT:    vcvtdq2ps %ymm0, %ymm0
; CHECK-WIDE-NEXT:    retl
  %res = uitofp <8 x i16> %src to <8 x float>
  ret <8 x float> %res
}

define <4 x float> @cvt_v4u8_v4f32(<4 x i8> %src) {
; CHECK-LABEL: cvt_v4u8_v4f32:
; CHECK:       ## %bb.0:
; CHECK-NEXT:    vpmovzxbd {{.*#+}} xmm0 = xmm0[0],zero,zero,zero,xmm0[1],zero,zero,zero,xmm0[2],zero,zero,zero,xmm0[3],zero,zero,zero
; CHECK-NEXT:    vcvtdq2ps %xmm0, %xmm0
; CHECK-NEXT:    retl
;
; CHECK-WIDE-LABEL: cvt_v4u8_v4f32:
; CHECK-WIDE:       ## %bb.0:
; CHECK-WIDE-NEXT:    vpmovzxbd {{.*#+}} xmm0 = xmm0[0],zero,zero,zero,xmm0[1],zero,zero,zero,xmm0[2],zero,zero,zero,xmm0[3],zero,zero,zero
; CHECK-WIDE-NEXT:    vcvtdq2ps %xmm0, %xmm0
; CHECK-WIDE-NEXT:    retl
  %res = uitofp <4 x i8> %src to <4 x float>
  ret <4 x float> %res
}

define <4 x float> @cvt_v4u16_v4f32(<4 x i16> %src) {
; CHECK-LABEL: cvt_v4u16_v4f32:
; CHECK:       ## %bb.0:
; CHECK-NEXT:    vpmovzxwd {{.*#+}} xmm0 = xmm0[0],zero,xmm0[1],zero,xmm0[2],zero,xmm0[3],zero
; CHECK-NEXT:    vcvtdq2ps %xmm0, %xmm0
; CHECK-NEXT:    retl
;
; CHECK-WIDE-LABEL: cvt_v4u16_v4f32:
; CHECK-WIDE:       ## %bb.0:
; CHECK-WIDE-NEXT:    vpmovzxwd {{.*#+}} xmm0 = xmm0[0],zero,xmm0[1],zero,xmm0[2],zero,xmm0[3],zero
; CHECK-WIDE-NEXT:    vcvtdq2ps %xmm0, %xmm0
; CHECK-WIDE-NEXT:    retl
  %res = uitofp <4 x i16> %src to <4 x float>
  ret <4 x float> %res
}

define <8 x i8> @cvt_v8f32_v8i8(<8 x float> %src) {
; CHECK-LABEL: cvt_v8f32_v8i8:
; CHECK:       ## %bb.0:
; CHECK-NEXT:    vcvttps2dq %ymm0, %ymm0
; CHECK-NEXT:    vextractf128 $1, %ymm0, %xmm1
; CHECK-NEXT:    vpackssdw %xmm1, %xmm0, %xmm0
; CHECK-NEXT:    vpacksswb %xmm0, %xmm0, %xmm0
; CHECK-NEXT:    vzeroupper
; CHECK-NEXT:    retl
;
; CHECK-WIDE-LABEL: cvt_v8f32_v8i8:
; CHECK-WIDE:       ## %bb.0:
; CHECK-WIDE-NEXT:    vcvttps2dq %ymm0, %ymm0
; CHECK-WIDE-NEXT:    vextractf128 $1, %ymm0, %xmm1
; CHECK-WIDE-NEXT:    vpackssdw %xmm1, %xmm0, %xmm0
; CHECK-WIDE-NEXT:    vpacksswb %xmm0, %xmm0, %xmm0
; CHECK-WIDE-NEXT:    vzeroupper
; CHECK-WIDE-NEXT:    retl
  %res = fptosi <8 x float> %src to <8 x i8>
  ret <8 x i8> %res
}

define <8 x i16> @cvt_v8f32_v8i16(<8 x float> %src) {
; CHECK-LABEL: cvt_v8f32_v8i16:
; CHECK:       ## %bb.0:
; CHECK-NEXT:    vcvttps2dq %ymm0, %ymm0
; CHECK-NEXT:    vextractf128 $1, %ymm0, %xmm1
; CHECK-NEXT:    vpackssdw %xmm1, %xmm0, %xmm0
; CHECK-NEXT:    vzeroupper
; CHECK-NEXT:    retl
;
; CHECK-WIDE-LABEL: cvt_v8f32_v8i16:
; CHECK-WIDE:       ## %bb.0:
; CHECK-WIDE-NEXT:    vcvttps2dq %ymm0, %ymm0
; CHECK-WIDE-NEXT:    vextractf128 $1, %ymm0, %xmm1
; CHECK-WIDE-NEXT:    vpackssdw %xmm1, %xmm0, %xmm0
; CHECK-WIDE-NEXT:    vzeroupper
; CHECK-WIDE-NEXT:    retl
  %res = fptosi <8 x float> %src to <8 x i16>
  ret <8 x i16> %res
}

define <4 x i8> @cvt_v4f32_v4i8(<4 x float> %src) {
; CHECK-LABEL: cvt_v4f32_v4i8:
; CHECK:       ## %bb.0:
; CHECK-NEXT:    vcvttps2dq %xmm0, %xmm0
; CHECK-NEXT:    vpshufb {{.*#+}} xmm0 = xmm0[0,4,8,12,u,u,u,u,u,u,u,u,u,u,u,u]
; CHECK-NEXT:    retl
;
; CHECK-WIDE-LABEL: cvt_v4f32_v4i8:
; CHECK-WIDE:       ## %bb.0:
; CHECK-WIDE-NEXT:    vcvttps2dq %xmm0, %xmm0
; CHECK-WIDE-NEXT:    vpshufb {{.*#+}} xmm0 = xmm0[0,4,8,12,u,u,u,u,u,u,u,u,u,u,u,u]
; CHECK-WIDE-NEXT:    retl
  %res = fptosi <4 x float> %src to <4 x i8>
  ret <4 x i8> %res
}

define <4 x i16> @cvt_v4f32_v4i16(<4 x float> %src) {
; CHECK-LABEL: cvt_v4f32_v4i16:
; CHECK:       ## %bb.0:
; CHECK-NEXT:    vcvttps2dq %xmm0, %xmm0
; CHECK-NEXT:    vpackssdw %xmm0, %xmm0, %xmm0
; CHECK-NEXT:    retl
;
; CHECK-WIDE-LABEL: cvt_v4f32_v4i16:
; CHECK-WIDE:       ## %bb.0:
; CHECK-WIDE-NEXT:    vcvttps2dq %xmm0, %xmm0
; CHECK-WIDE-NEXT:    vpackssdw %xmm0, %xmm0, %xmm0
; CHECK-WIDE-NEXT:    retl
  %res = fptosi <4 x float> %src to <4 x i16>
  ret <4 x i16> %res
}

define <8 x i8> @cvt_v8f32_v8u8(<8 x float> %src) {
; CHECK-LABEL: cvt_v8f32_v8u8:
; CHECK:       ## %bb.0:
; CHECK-NEXT:    vcvttps2dq %ymm0, %ymm0
; CHECK-NEXT:    vextractf128 $1, %ymm0, %xmm1
; CHECK-NEXT:    vpackssdw %xmm1, %xmm0, %xmm0
; CHECK-NEXT:    vpackuswb %xmm0, %xmm0, %xmm0
; CHECK-NEXT:    vzeroupper
; CHECK-NEXT:    retl
;
; CHECK-WIDE-LABEL: cvt_v8f32_v8u8:
; CHECK-WIDE:       ## %bb.0:
; CHECK-WIDE-NEXT:    vcvttps2dq %ymm0, %ymm0
; CHECK-WIDE-NEXT:    vextractf128 $1, %ymm0, %xmm1
; CHECK-WIDE-NEXT:    vpackssdw %xmm1, %xmm0, %xmm0
; CHECK-WIDE-NEXT:    vpackuswb %xmm0, %xmm0, %xmm0
; CHECK-WIDE-NEXT:    vzeroupper
; CHECK-WIDE-NEXT:    retl
  %res = fptoui <8 x float> %src to <8 x i8>
  ret <8 x i8> %res
}

define <8 x i16> @cvt_v8f32_v8u16(<8 x float> %src) {
; CHECK-LABEL: cvt_v8f32_v8u16:
; CHECK:       ## %bb.0:
; CHECK-NEXT:    vcvttps2dq %ymm0, %ymm0
; CHECK-NEXT:    vextractf128 $1, %ymm0, %xmm1
; CHECK-NEXT:    vpackusdw %xmm1, %xmm0, %xmm0
; CHECK-NEXT:    vzeroupper
; CHECK-NEXT:    retl
;
; CHECK-WIDE-LABEL: cvt_v8f32_v8u16:
; CHECK-WIDE:       ## %bb.0:
; CHECK-WIDE-NEXT:    vcvttps2dq %ymm0, %ymm0
; CHECK-WIDE-NEXT:    vextractf128 $1, %ymm0, %xmm1
; CHECK-WIDE-NEXT:    vpackusdw %xmm1, %xmm0, %xmm0
; CHECK-WIDE-NEXT:    vzeroupper
; CHECK-WIDE-NEXT:    retl
  %res = fptoui <8 x float> %src to <8 x i16>
  ret <8 x i16> %res
}

define <4 x i8> @cvt_v4f32_v4u8(<4 x float> %src) {
; CHECK-LABEL: cvt_v4f32_v4u8:
; CHECK:       ## %bb.0:
; CHECK-NEXT:    vcvttps2dq %xmm0, %xmm0
; CHECK-NEXT:    vpshufb {{.*#+}} xmm0 = xmm0[0,4,8,12,u,u,u,u,u,u,u,u,u,u,u,u]
; CHECK-NEXT:    retl
;
; CHECK-WIDE-LABEL: cvt_v4f32_v4u8:
; CHECK-WIDE:       ## %bb.0:
; CHECK-WIDE-NEXT:    vcvttps2dq %xmm0, %xmm0
; CHECK-WIDE-NEXT:    vpshufb {{.*#+}} xmm0 = xmm0[0,4,8,12,u,u,u,u,u,u,u,u,u,u,u,u]
; CHECK-WIDE-NEXT:    retl
  %res = fptoui <4 x float> %src to <4 x i8>
  ret <4 x i8> %res
}

define <4 x i16> @cvt_v4f32_v4u16(<4 x float> %src) {
; CHECK-LABEL: cvt_v4f32_v4u16:
; CHECK:       ## %bb.0:
; CHECK-NEXT:    vcvttps2dq %xmm0, %xmm0
; CHECK-NEXT:    vpackusdw %xmm0, %xmm0, %xmm0
; CHECK-NEXT:    retl
;
; CHECK-WIDE-LABEL: cvt_v4f32_v4u16:
; CHECK-WIDE:       ## %bb.0:
; CHECK-WIDE-NEXT:    vcvttps2dq %xmm0, %xmm0
; CHECK-WIDE-NEXT:    vpackusdw %xmm0, %xmm0, %xmm0
; CHECK-WIDE-NEXT:    retl
  %res = fptoui <4 x float> %src to <4 x i16>
  ret <4 x i16> %res
}

