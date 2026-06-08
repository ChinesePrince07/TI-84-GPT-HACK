// TI-84 Token Decoder
// Maps TI-84 tokens to readable text for GPT

#ifndef TI_TOKENS_H
#define TI_TOKENS_H

#include <Arduino.h>

// Single-byte tokens (0x00 - 0xFF)
// Only including tokens that need special handling (not plain ASCII)
struct TokenEntry {
  uint8_t token;
  const char* text;
};

// Two-byte tokens (prefix byte + second byte)
struct Token2Entry {
  uint8_t prefix;
  uint8_t token;
  const char* text;
};

// Common single-byte tokens
const TokenEntry singleTokens[] = {
  // Math operations
  { 0x10, "(" },
  { 0x11, ")" },
  { 0x2A, "\"" },
  { 0x2B, "," },
  { 0x2C, "i" },        // imaginary i
  { 0x2D, "!" },        // factorial
  { 0x3A, "." },
  { 0x3B, "E" },        // scientific notation
  { 0x3C, " or " },
  { 0x3D, " xor " },
  { 0x3E, ":" },
  { 0x3F, "\n" },       // newline/statement separator
  { 0x40, " and " },
  { 0x41, "A" }, { 0x42, "B" }, { 0x43, "C" }, { 0x44, "D" }, { 0x45, "E" },
  { 0x46, "F" }, { 0x47, "G" }, { 0x48, "H" }, { 0x49, "I" }, { 0x4A, "J" },
  { 0x4B, "K" }, { 0x4C, "L" }, { 0x4D, "M" }, { 0x4E, "N" }, { 0x4F, "O" },
  { 0x50, "P" }, { 0x51, "Q" }, { 0x52, "R" }, { 0x53, "S" }, { 0x54, "T" },
  { 0x55, "U" }, { 0x56, "V" }, { 0x57, "W" }, { 0x58, "X" }, { 0x59, "Y" },
  { 0x5A, "Z" },
  { 0x5B, "theta" },

  // Math symbols and functions (0x6A - 0x9F)
  { 0x6A, "=" },
  { 0x6B, "<" },
  { 0x6C, ">" },
  { 0x6D, "<=" },
  { 0x6E, ">=" },
  { 0x6F, "!=" },
  { 0x70, "+" },
  { 0x71, "-" },
  { 0x72, "Ans" },
  { 0x73, "Fix " },
  { 0x74, "Horiz" },
  { 0x80, "+" },        // add
  { 0x81, "-" },        // subtract
  { 0x82, "*" },        // multiply
  { 0x83, "/" },        // divide
  { 0x84, "^" },        // power
  { 0x85, "xroot(" },   // x root
  { 0x86, "frac(" },
  { 0x87, "sqrt(" },    // legacy sqrt
  { 0x88, "cubert(" },  // cube root
  // Exponents
  { 0x0C, "^-1" },      // inverse ⁻¹
  { 0x0D, "^2" },       // squared ²

  // More functions
  { 0xB0, "-" },        // negative sign
  { 0xB1, "int(" },
  { 0xB2, "abs(" },
  { 0xB3, "det(" },
  { 0xB4, "identity(" },
  { 0xB5, "dim(" },
  { 0xB6, "sum(" },
  { 0xB7, "prod(" },
  { 0xB8, "not(" },
  { 0xB9, "iPart(" },
  { 0xBA, "fPart(" },
  { 0xBC, "sqrt(" },    // square root √(
  { 0xBD, "cubert(" },  // cube root ³√(
  { 0xBE, "ln(" },
  { 0xBF, "e^(" },
  { 0xC0, "log(" },
  { 0xC1, "10^(" },

  // Trig functions (TI-84 tokens - interleaved with inverses)
  { 0xC2, "sin(" },
  { 0xC3, "arcsin(" },  // sin⁻¹(
  { 0xC4, "cos(" },
  { 0xC5, "arccos(" },  // cos⁻¹(
  { 0xC6, "tan(" },
  { 0xC7, "arctan(" },  // tan⁻¹(
  { 0xC8, "sinh(" },
  { 0xC9, "arcsinh(" }, // sinh⁻¹(
  { 0xCA, "cosh(" },
  { 0xCB, "arccosh(" }, // cosh⁻¹(
  { 0xCC, "tanh(" },
  { 0xCD, "arctanh(" }, // tanh⁻¹(

  // Calculus tokens
  { 0xF0, "nDeriv(" },   // numerical derivative
  { 0xF1, "fnInt(" },    // function integral

  { 0x00, NULL }  // end marker
};

// Two-byte tokens with 0xBB prefix (extended math functions)
const Token2Entry bbTokens[] = {
  // Calculus and advanced math
  { 0xBB, 0x00, "npv(" },
  { 0xBB, 0x01, "irr(" },
  { 0xBB, 0x02, "bal(" },
  { 0xBB, 0x03, "Sigma*Sigma(" },
  { 0xBB, 0x04, "stdDev(" },
  { 0xBB, 0x05, "variance(" },
  { 0xBB, 0x06, "inString(" },
  { 0xBB, 0x07, "normalcdf(" },
  { 0xBB, 0x08, "invNorm(" },
  { 0xBB, 0x09, "tcdf(" },
  { 0xBB, 0x0A, "X2cdf(" },
  { 0xBB, 0x0B, "Fcdf(" },
  { 0xBB, 0x0C, "binompdf(" },
  { 0xBB, 0x0D, "binomcdf(" },
  { 0xBB, 0x0E, "poissonpdf(" },
  { 0xBB, 0x0F, "poissoncdf(" },
  { 0xBB, 0x10, "geometpdf(" },
  { 0xBB, 0x11, "geometcdf(" },
  { 0xBB, 0x12, "normalpdf(" },
  { 0xBB, 0x13, "tpdf(" },
  { 0xBB, 0x14, "X2pdf(" },
  { 0xBB, 0x15, "Fpdf(" },
  { 0xBB, 0x16, "randNorm(" },
  { 0xBB, 0x17, "randBin(" },
  { 0xBB, 0x18, "randInt(" },
  { 0xBB, 0x19, "randIntNoRep(" },
  { 0xBB, 0x1A, "randM(" },
  { 0xBB, 0x1B, "randBin(" },
  { 0xBB, 0x1C, "mean(" },
  { 0xBB, 0x1D, "median(" },
  { 0xBB, 0x1E, "solve(" },
  { 0xBB, 0x1F, "seq(" },
  { 0xBB, 0x20, "fnInt(" },      // integral
  { 0xBB, 0x21, "nDeriv(" },     // derivative
  { 0xBB, 0x22, "fMin(" },
  { 0xBB, 0x23, "fMax(" },

  // Templates (MathPrint style)
  { 0xBB, 0x2C, "d/dx(" },       // derivative template
  { 0xBB, 0x2D, "(d/d_)(" },     // derivative with variable
  { 0xBB, 0x2E, "integral(" },   // integral template ∫
  { 0xBB, 0x2F, "integral(" },   // definite integral
  { 0xBB, 0x30, "sum(" },        // summation Σ
  { 0xBB, 0x31, "product(" },    // product Π
  { 0xBB, 0x32, "nthroot(" },    // nth root
  { 0xBB, 0x33, "logbase(" },    // log with base
  { 0xBB, 0x34, "cubert(" },     // cube root
  { 0xBB, 0x35, "sqrt(" },       // square root template

  // Superscripts/subscripts
  { 0xBB, 0x36, "^2" },          // squared
  { 0xBB, 0x37, "^3" },          // cubed
  { 0xBB, 0x38, "^-1" },         // inverse

  // Fractions
  { 0xBB, 0x3A, "/" },           // fraction
  { 0xBB, 0x3B, "mixedFrac(" },

  // More functions
  { 0xBB, 0x40, "piecewise(" },
  { 0xBB, 0x51, "|" },           // "such that" in set builder
  { 0xBB, 0x52, "e" },           // constant e

  { 0x00, 0x00, NULL }  // end marker
};

// Two-byte tokens with 0x5C prefix (matrices)
const Token2Entry matrixTokens[] = {
  { 0x5C, 0x00, "[A]" },
  { 0x5C, 0x01, "[B]" },
  { 0x5C, 0x02, "[C]" },
  { 0x5C, 0x03, "[D]" },
  { 0x5C, 0x04, "[E]" },
  { 0x5C, 0x05, "[F]" },
  { 0x5C, 0x06, "[G]" },
  { 0x5C, 0x07, "[H]" },
  { 0x5C, 0x08, "[I]" },
  { 0x5C, 0x09, "[J]" },
  { 0x00, 0x00, NULL }
};

// Two-byte tokens with 0x5D prefix (lists)
const Token2Entry listTokens[] = {
  { 0x5D, 0x00, "L1" },
  { 0x5D, 0x01, "L2" },
  { 0x5D, 0x02, "L3" },
  { 0x5D, 0x03, "L4" },
  { 0x5D, 0x04, "L5" },
  { 0x5D, 0x05, "L6" },
  { 0x00, 0x00, NULL }
};

// Two-byte tokens with 0x5E prefix (equations)
const Token2Entry eqTokens[] = {
  { 0x5E, 0x10, "Y1" },
  { 0x5E, 0x11, "Y2" },
  { 0x5E, 0x12, "Y3" },
  { 0x5E, 0x13, "Y4" },
  { 0x5E, 0x14, "Y5" },
  { 0x5E, 0x15, "Y6" },
  { 0x5E, 0x16, "Y7" },
  { 0x5E, 0x17, "Y8" },
  { 0x5E, 0x18, "Y9" },
  { 0x5E, 0x19, "Y0" },
  { 0x5E, 0x20, "X1T" },
  { 0x5E, 0x21, "Y1T" },
  { 0x5E, 0x22, "X2T" },
  { 0x5E, 0x23, "Y2T" },
  { 0x5E, 0x40, "r1" },
  { 0x5E, 0x41, "r2" },
  { 0x5E, 0x42, "r3" },
  { 0x5E, 0x43, "r4" },
  { 0x5E, 0x44, "r5" },
  { 0x5E, 0x45, "r6" },
  { 0x5E, 0x80, "u" },
  { 0x5E, 0x81, "v" },
  { 0x5E, 0x82, "w" },
  { 0x00, 0x00, NULL }
};

// Two-byte tokens with 0x63 prefix (system variables)
const Token2Entry sysTokens[] = {
  { 0x63, 0x00, "RegEQ" },
  { 0x63, 0x01, "n" },
  { 0x63, 0x02, "xbar" },
  { 0x63, 0x03, "sumX" },
  { 0x63, 0x04, "sumX2" },
  { 0x63, 0x05, "Sx" },
  { 0x63, 0x06, "sigmax" },
  { 0x63, 0x07, "minX" },
  { 0x63, 0x08, "maxX" },
  { 0x63, 0x09, "minY" },
  { 0x63, 0x0A, "maxY" },
  { 0x63, 0x0B, "ybar" },
  { 0x63, 0x0C, "sumY" },
  { 0x63, 0x0D, "sumY2" },
  { 0x63, 0x0E, "Sy" },
  { 0x63, 0x0F, "sigmay" },
  { 0x63, 0x10, "sumXY" },
  { 0x63, 0x11, "r" },
  { 0x63, 0x12, "Med" },
  { 0x63, 0x13, "Q1" },
  { 0x63, 0x14, "Q3" },
  { 0x00, 0x00, NULL }
};

// Greek letters and special symbols with 0xBB prefix
const Token2Entry greekTokens[] = {
  { 0xBB, 0xCE, "alpha" },
  { 0xBB, 0xCF, "beta" },
  { 0xBB, 0xD0, "gamma" },
  { 0xBB, 0xD1, "DELTA" },
  { 0xBB, 0xD2, "delta" },
  { 0xBB, 0xD3, "epsilon" },
  { 0xBB, 0xD4, "lambda" },
  { 0xBB, 0xD5, "mu" },
  { 0xBB, 0xD6, "pi" },
  { 0xBB, 0xD7, "rho" },
  { 0xBB, 0xD8, "SIGMA" },
  { 0xBB, 0xD9, "sigma" },
  { 0xBB, 0xDA, "tau" },
  { 0xBB, 0xDB, "phi" },
  { 0xBB, 0xDC, "OMEGA" },
  { 0xBB, 0xDD, "omega" },
  { 0x00, 0x00, NULL }
};

// Check if a byte is a two-byte token prefix
bool isTwoBytePrefix(uint8_t b) {
  return (b == 0x5C || b == 0x5D || b == 0x5E ||
          b == 0x60 || b == 0x62 || b == 0x63 ||
          b == 0x7E || b == 0xAA || b == 0xBB || b == 0xEF);
}

// Lookup a single-byte token
const char* lookupSingleToken(uint8_t token) {
  for (int i = 0; singleTokens[i].text != NULL; i++) {
    if (singleTokens[i].token == token) {
      return singleTokens[i].text;
    }
  }
  return NULL;
}

// Lookup a two-byte token in a specific table
const char* lookupInTable(const Token2Entry* table, uint8_t prefix, uint8_t token) {
  for (int i = 0; table[i].text != NULL; i++) {
    if (table[i].prefix == prefix && table[i].token == token) {
      return table[i].text;
    }
  }
  return NULL;
}

// Lookup a two-byte token
const char* lookupTwoByteToken(uint8_t prefix, uint8_t token) {
  const char* result = NULL;

  switch (prefix) {
    case 0x5C:
      result = lookupInTable(matrixTokens, prefix, token);
      break;
    case 0x5D:
      result = lookupInTable(listTokens, prefix, token);
      break;
    case 0x5E:
      result = lookupInTable(eqTokens, prefix, token);
      break;
    case 0x63:
      result = lookupInTable(sysTokens, prefix, token);
      break;
    case 0xBB:
      result = lookupInTable(bbTokens, prefix, token);
      if (result == NULL) {
        result = lookupInTable(greekTokens, prefix, token);
      }
      break;
    case 0xAA:
      // String variables Str0-Str9
      if (token <= 9) {
        static char strBuf[8];
        snprintf(strBuf, sizeof(strBuf), "Str%d", token);
        return strBuf;
      }
      break;
    case 0x60:
      // Picture variables Pic0-Pic9
      if (token <= 9) {
        static char picBuf[8];
        snprintf(picBuf, sizeof(picBuf), "Pic%d", token);
        return picBuf;
      }
      break;
  }

  return result;
}

// Decode a TI token string to readable text
// Input: raw token bytes from TI-84
// Output: decoded text in result buffer
// Returns: length of decoded text
int decodeTokenString(const uint8_t* tokens, int tokenLen, char* result, int resultMaxLen) {
  int resultLen = 0;
  int i = 0;

  while (i < tokenLen && resultLen < resultMaxLen - 1) {
    uint8_t b = tokens[i];

    // Check for two-byte token
    if (isTwoBytePrefix(b) && i + 1 < tokenLen) {
      uint8_t b2 = tokens[i + 1];
      const char* text = lookupTwoByteToken(b, b2);
      if (text != NULL) {
        int len = strlen(text);
        if (resultLen + len < resultMaxLen) {
          strcpy(result + resultLen, text);
          resultLen += len;
        }
        i += 2;
        continue;
      }
      // Unknown two-byte token, skip both bytes
      i += 2;
      continue;
    }

    // Check for single-byte token
    const char* text = lookupSingleToken(b);
    if (text != NULL) {
      int len = strlen(text);
      if (resultLen + len < resultMaxLen) {
        strcpy(result + resultLen, text);
        resultLen += len;
      }
      i++;
      continue;
    }

    // ASCII printable characters (0x20-0x7E)
    if (b >= 0x20 && b <= 0x7E) {
      result[resultLen++] = (char)b;
      i++;
      continue;
    }

    // Numbers 0-9 (0x30-0x39)
    if (b >= 0x30 && b <= 0x39) {
      result[resultLen++] = (char)b;
      i++;
      continue;
    }

    // Unknown token - skip
    i++;
  }

  result[resultLen] = '\0';
  return resultLen;
}

#endif // TI_TOKENS_H
