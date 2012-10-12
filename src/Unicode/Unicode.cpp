//
//  Unicode.cpp
//  This file is part of the "Kai" project, and is released under the MIT license.
//
//  Created by Samuel Williams on 7/12/11.
//  Copyright (c) 2011 Orion Transfer Ltd. All rights reserved.
//

#include "Unicode.h"

namespace Kai {
	namespace Unicode {
		bool in_range(const CodePointT & code_point, const CodePointT & min, const CodePointT & max)
		{
			return code_point >= min && code_point <= max;
		}
		
		bool is_letter(CodePointT cp)
		{
			return in_range(cp, 0x0041, 0x005A) || in_range(cp, 0x0061, 0x007A) || cp == 0x00AA || cp == 0x00B5 || cp == 0x00BA || in_range(cp, 0x00C0, 0x00D6) || in_range(cp, 0x00D8, 0x00F6) || in_range(cp, 0x00F8, 0x02C1) || in_range(cp, 0x02C6, 0x02D1) || in_range(cp, 0x02E0, 0x02E4) || cp == 0x02EC || cp == 0x02EE || in_range(cp, 0x0370, 0x0374) || cp == 0x0376 || cp == 0x0377 || in_range(cp, 0x037A, 0x037D) || cp == 0x0386 || in_range(cp, 0x0388, 0x038A) || cp == 0x038C || in_range(cp, 0x038E, 0x03A1) || in_range(cp, 0x03A3, 0x03F5) || in_range(cp, 0x03F7, 0x0481) || in_range(cp, 0x048A, 0x0525) || in_range(cp, 0x0531, 0x0556) || cp == 0x0559 || in_range(cp, 0x0561, 0x0587) || in_range(cp, 0x05D0, 0x05EA) || in_range(cp, 0x05F0, 0x05F2) || in_range(cp, 0x0621, 0x064A) || cp == 0x066E || cp == 0x066F || in_range(cp, 0x0671, 0x06D3) || cp == 0x06D5 || cp == 0x06E5 || cp == 0x06E6 || cp == 0x06EE || cp == 0x06EF || in_range(cp, 0x06FA, 0x06FC) || cp == 0x06FF || cp == 0x0710 || in_range(cp, 0x0712, 0x072F) || in_range(cp, 0x074D, 0x07A5) || cp == 0x07B1 || in_range(cp, 0x07CA, 0x07EA) || cp == 0x07F4 || cp == 0x07F5 || cp == 0x07FA || in_range(cp, 0x0800, 0x0815) || cp == 0x081A || cp == 0x0824 || cp == 0x0828 || in_range(cp, 0x0904, 0x0939) || cp == 0x093D || cp == 0x0950 || in_range(cp, 0x0958, 0x0961) || cp == 0x0971 || cp == 0x0972 || in_range(cp, 0x0979, 0x097F) || in_range(cp, 0x0985, 0x098C) || cp == 0x098F || cp == 0x0990 || in_range(cp, 0x0993, 0x09A8) || in_range(cp, 0x09AA, 0x09B0) || cp == 0x09B2 || in_range(cp, 0x09B6, 0x09B9) || cp == 0x09BD || cp == 0x09CE || cp == 0x09DC || cp == 0x09DD || in_range(cp, 0x09DF, 0x09E1) || cp == 0x09F0 || cp == 0x09F1 || in_range(cp, 0x0A05, 0x0A0A) || cp == 0x0A0F || cp == 0x0A10 || in_range(cp, 0x0A13, 0x0A28) || in_range(cp, 0x0A2A, 0x0A30) || cp == 0x0A32 || cp == 0x0A33 || cp == 0x0A35 || cp == 0x0A36 || cp == 0x0A38 || cp == 0x0A39 || in_range(cp, 0x0A59, 0x0A5C) || cp == 0x0A5E || in_range(cp, 0x0A72, 0x0A74) || in_range(cp, 0x0A85, 0x0A8D) || in_range(cp, 0x0A8F, 0x0A91) || in_range(cp, 0x0A93, 0x0AA8) || in_range(cp, 0x0AAA, 0x0AB0) || cp == 0x0AB2 || cp == 0x0AB3 || in_range(cp, 0x0AB5, 0x0AB9) || cp == 0x0ABD || cp == 0x0AD0 || cp == 0x0AE0 || cp == 0x0AE1 || in_range(cp, 0x0B05, 0x0B0C) || cp == 0x0B0F || cp == 0x0B10 || in_range(cp, 0x0B13, 0x0B28) || in_range(cp, 0x0B2A, 0x0B30) || cp == 0x0B32 || cp == 0x0B33 || in_range(cp, 0x0B35, 0x0B39) || cp == 0x0B3D || cp == 0x0B5C || cp == 0x0B5D || in_range(cp, 0x0B5F, 0x0B61) || cp == 0x0B71 || cp == 0x0B83 || in_range(cp, 0x0B85, 0x0B8A) || in_range(cp, 0x0B8E, 0x0B90) || in_range(cp, 0x0B92, 0x0B95) || cp == 0x0B99 || cp == 0x0B9A || cp == 0x0B9C || cp == 0x0B9E || cp == 0x0B9F || cp == 0x0BA3 || cp == 0x0BA4 || in_range(cp, 0x0BA8, 0x0BAA) || in_range(cp, 0x0BAE, 0x0BB9) || cp == 0x0BD0 || in_range(cp, 0x0C05, 0x0C0C) || in_range(cp, 0x0C0E, 0x0C10) || in_range(cp, 0x0C12, 0x0C28) || in_range(cp, 0x0C2A, 0x0C33) || in_range(cp, 0x0C35, 0x0C39) || cp == 0x0C3D || cp == 0x0C58 || cp == 0x0C59 || cp == 0x0C60 || cp == 0x0C61 || in_range(cp, 0x0C85, 0x0C8C) || in_range(cp, 0x0C8E, 0x0C90) || in_range(cp, 0x0C92, 0x0CA8) || in_range(cp, 0x0CAA, 0x0CB3) || in_range(cp, 0x0CB5, 0x0CB9) || cp == 0x0CBD || cp == 0x0CDE || cp == 0x0CE0 || cp == 0x0CE1 || in_range(cp, 0x0D05, 0x0D0C) || in_range(cp, 0x0D0E, 0x0D10) || in_range(cp, 0x0D12, 0x0D28) || in_range(cp, 0x0D2A, 0x0D39) || cp == 0x0D3D || cp == 0x0D60 || cp == 0x0D61 || in_range(cp, 0x0D7A, 0x0D7F) || in_range(cp, 0x0D85, 0x0D96) || in_range(cp, 0x0D9A, 0x0DB1) || in_range(cp, 0x0DB3, 0x0DBB) || cp == 0x0DBD || in_range(cp, 0x0DC0, 0x0DC6) || in_range(cp, 0x0E01, 0x0E30) || cp == 0x0E32 || cp == 0x0E33 || in_range(cp, 0x0E40, 0x0E46) || cp == 0x0E81 || cp == 0x0E82 || cp == 0x0E84 || cp == 0x0E87 || cp == 0x0E88 || cp == 0x0E8A || cp == 0x0E8D || in_range(cp, 0x0E94, 0x0E97) || in_range(cp, 0x0E99, 0x0E9F) || in_range(cp, 0x0EA1, 0x0EA3) || cp == 0x0EA5 || cp == 0x0EA7 || cp == 0x0EAA || cp == 0x0EAB || in_range(cp, 0x0EAD, 0x0EB0) || cp == 0x0EB2 || cp == 0x0EB3 || cp == 0x0EBD || in_range(cp, 0x0EC0, 0x0EC4) || cp == 0x0EC6 || cp == 0x0EDC || cp == 0x0EDD || cp == 0x0F00 || in_range(cp, 0x0F40, 0x0F47) || in_range(cp, 0x0F49, 0x0F6C) || in_range(cp, 0x0F88, 0x0F8B) || in_range(cp, 0x1000, 0x102A) || cp == 0x103F || in_range(cp, 0x1050, 0x1055) || in_range(cp, 0x105A, 0x105D) || cp == 0x1061 || cp == 0x1065 || cp == 0x1066 || in_range(cp, 0x106E, 0x1070) || in_range(cp, 0x1075, 0x1081) || cp == 0x108E || in_range(cp, 0x10A0, 0x10C5) || in_range(cp, 0x10D0, 0x10FA) || cp == 0x10FC || in_range(cp, 0x1100, 0x1248) || in_range(cp, 0x124A, 0x124D) || in_range(cp, 0x1250, 0x1256) || cp == 0x1258 || in_range(cp, 0x125A, 0x125D) || in_range(cp, 0x1260, 0x1288) || in_range(cp, 0x128A, 0x128D) || in_range(cp, 0x1290, 0x12B0) || in_range(cp, 0x12B2, 0x12B5) || in_range(cp, 0x12B8, 0x12BE) || cp == 0x12C0 || in_range(cp, 0x12C2, 0x12C5) || in_range(cp, 0x12C8, 0x12D6) || in_range(cp, 0x12D8, 0x1310) || in_range(cp, 0x1312, 0x1315) || in_range(cp, 0x1318, 0x135A) || in_range(cp, 0x1380, 0x138F) || in_range(cp, 0x13A0, 0x13F4) || in_range(cp, 0x1401, 0x166C) || in_range(cp, 0x166F, 0x167F) || in_range(cp, 0x1681, 0x169A) || in_range(cp, 0x16A0, 0x16EA) || in_range(cp, 0x1700, 0x170C) || in_range(cp, 0x170E, 0x1711) || in_range(cp, 0x1720, 0x1731) || in_range(cp, 0x1740, 0x1751) || in_range(cp, 0x1760, 0x176C) || in_range(cp, 0x176E, 0x1770) || in_range(cp, 0x1780, 0x17B3) || cp == 0x17D7 || cp == 0x17DC || in_range(cp, 0x1820, 0x1877) || in_range(cp, 0x1880, 0x18A8) || cp == 0x18AA || in_range(cp, 0x18B0, 0x18F5) || in_range(cp, 0x1900, 0x191C) || in_range(cp, 0x1950, 0x196D) || in_range(cp, 0x1970, 0x1974) || in_range(cp, 0x1980, 0x19AB) || in_range(cp, 0x19C1, 0x19C7) || in_range(cp, 0x1A00, 0x1A16) || in_range(cp, 0x1A20, 0x1A54) || cp == 0x1AA7 || in_range(cp, 0x1B05, 0x1B33) || in_range(cp, 0x1B45, 0x1B4B) || in_range(cp, 0x1B83, 0x1BA0) || cp == 0x1BAE || cp == 0x1BAF || in_range(cp, 0x1C00, 0x1C23) || in_range(cp, 0x1C4D, 0x1C4F) || in_range(cp, 0x1C5A, 0x1C7D) || in_range(cp, 0x1CE9, 0x1CEC) || in_range(cp, 0x1CEE, 0x1CF1) || in_range(cp, 0x1D00, 0x1DBF) || in_range(cp, 0x1E00, 0x1F15) || in_range(cp, 0x1F18, 0x1F1D) || in_range(cp, 0x1F20, 0x1F45) || in_range(cp, 0x1F48, 0x1F4D) || in_range(cp, 0x1F50, 0x1F57) || cp == 0x1F59 || cp == 0x1F5B || cp == 0x1F5D || in_range(cp, 0x1F5F, 0x1F7D) || in_range(cp, 0x1F80, 0x1FB4) || in_range(cp, 0x1FB6, 0x1FBC) || cp == 0x1FBE || in_range(cp, 0x1FC2, 0x1FC4) || in_range(cp, 0x1FC6, 0x1FCC) || in_range(cp, 0x1FD0, 0x1FD3) || in_range(cp, 0x1FD6, 0x1FDB) || in_range(cp, 0x1FE0, 0x1FEC) || in_range(cp, 0x1FF2, 0x1FF4) || in_range(cp, 0x1FF6, 0x1FFC) || cp == 0x2071 || cp == 0x207F || in_range(cp, 0x2090, 0x2094) || cp == 0x2102 || cp == 0x2107 || in_range(cp, 0x210A, 0x2113) || cp == 0x2115 || in_range(cp, 0x2119, 0x211D) || cp == 0x2124 || cp == 0x2126 || cp == 0x2128 || in_range(cp, 0x212A, 0x212D) || in_range(cp, 0x212F, 0x2139) || in_range(cp, 0x213C, 0x213F) || in_range(cp, 0x2145, 0x2149) || cp == 0x214E || cp == 0x2183 || cp == 0x2184 || in_range(cp, 0x2C00, 0x2C2E) || in_range(cp, 0x2C30, 0x2C5E) || in_range(cp, 0x2C60, 0x2CE4) || in_range(cp, 0x2CEB, 0x2CEE) || in_range(cp, 0x2D00, 0x2D25) || in_range(cp, 0x2D30, 0x2D65) || cp == 0x2D6F || in_range(cp, 0x2D80, 0x2D96) || in_range(cp, 0x2DA0, 0x2DA6) || in_range(cp, 0x2DA8, 0x2DAE) || in_range(cp, 0x2DB0, 0x2DB6) || in_range(cp, 0x2DB8, 0x2DBE) || in_range(cp, 0x2DC0, 0x2DC6) || in_range(cp, 0x2DC8, 0x2DCE) || in_range(cp, 0x2DD0, 0x2DD6) || in_range(cp, 0x2DD8, 0x2DDE) || cp == 0x2E2F || cp == 0x3005 || cp == 0x3006 || in_range(cp, 0x3031, 0x3035) || cp == 0x303B || cp == 0x303C || in_range(cp, 0x3041, 0x3096) || in_range(cp, 0x309D, 0x309F) || in_range(cp, 0x30A1, 0x30FA) || in_range(cp, 0x30FC, 0x30FF) || in_range(cp, 0x3105, 0x312D) || in_range(cp, 0x3131, 0x318E) || in_range(cp, 0x31A0, 0x31B7) || in_range(cp, 0x31F0, 0x31FF) || in_range(cp, 0x3400, 0x4DB5) || in_range(cp, 0x4E00, 0x9FCB) || in_range(cp, 0xA000, 0xA48C) || in_range(cp, 0xA4D0, 0xA4FD) || in_range(cp, 0xA500, 0xA60C) || in_range(cp, 0xA610, 0xA61F) || cp == 0xA62A || cp == 0xA62B || in_range(cp, 0xA640, 0xA65F) || in_range(cp, 0xA662, 0xA66E) || in_range(cp, 0xA67F, 0xA697) || in_range(cp, 0xA6A0, 0xA6E5) || in_range(cp, 0xA717, 0xA71F) || in_range(cp, 0xA722, 0xA788) || cp == 0xA78B || cp == 0xA78C || in_range(cp, 0xA7FB, 0xA801) || in_range(cp, 0xA803, 0xA805) || in_range(cp, 0xA807, 0xA80A) || in_range(cp, 0xA80C, 0xA822) || in_range(cp, 0xA840, 0xA873) || in_range(cp, 0xA882, 0xA8B3) || in_range(cp, 0xA8F2, 0xA8F7) || cp == 0xA8FB || in_range(cp, 0xA90A, 0xA925) || in_range(cp, 0xA930, 0xA946) || in_range(cp, 0xA960, 0xA97C) || in_range(cp, 0xA984, 0xA9B2) || cp == 0xA9CF || in_range(cp, 0xAA00, 0xAA28) || in_range(cp, 0xAA40, 0xAA42) || in_range(cp, 0xAA44, 0xAA4B) || in_range(cp, 0xAA60, 0xAA76) || cp == 0xAA7A || in_range(cp, 0xAA80, 0xAAAF) || cp == 0xAAB1 || cp == 0xAAB5 || cp == 0xAAB6 || in_range(cp, 0xAAB9, 0xAABD) || cp == 0xAAC0 || cp == 0xAAC2 || in_range(cp, 0xAADB, 0xAADD) || in_range(cp, 0xABC0, 0xABE2) || in_range(cp, 0xAC00, 0xD7A3) || in_range(cp, 0xD7B0, 0xD7C6) || in_range(cp, 0xD7CB, 0xD7FB) || in_range(cp, 0xF900, 0xFA2D) || in_range(cp, 0xFA30, 0xFA6D) || in_range(cp, 0xFA70, 0xFAD9) || in_range(cp, 0xFB00, 0xFB06) || in_range(cp, 0xFB13, 0xFB17) || cp == 0xFB1D || in_range(cp, 0xFB1F, 0xFB28) || in_range(cp, 0xFB2A, 0xFB36) || in_range(cp, 0xFB38, 0xFB3C) || cp == 0xFB3E || cp == 0xFB40 || cp == 0xFB41 || cp == 0xFB43 || cp == 0xFB44 || in_range(cp, 0xFB46, 0xFBB1) || in_range(cp, 0xFBD3, 0xFD3D) || in_range(cp, 0xFD50, 0xFD8F) || in_range(cp, 0xFD92, 0xFDC7) || in_range(cp, 0xFDF0, 0xFDFB) || in_range(cp, 0xFE70, 0xFE74) || in_range(cp, 0xFE76, 0xFEFC) || in_range(cp, 0xFF21, 0xFF3A) || in_range(cp, 0xFF41, 0xFF5A) || in_range(cp, 0xFF66, 0xFFBE) || in_range(cp, 0xFFC2, 0xFFC7) || in_range(cp, 0xFFCA, 0xFFCF) || in_range(cp, 0xFFD2, 0xFFD7) || in_range(cp, 0xFFDA, 0xFFDC);
		}
		
		bool is_alpha(CodePointT code_point) {
			return in_range(code_point, 'a', 'z') || in_range(code_point, 'A', 'Z') || code_point == '_';
		}
		
		bool is_numeric(CodePointT code_point) {
			return in_range(code_point, '0', '9');
		}
		
		bool is_alpha_numeric(CodePointT code_point) {
			return is_alpha(code_point) || is_numeric(code_point);
		}
		
		bool is_hexadecimal(CodePointT code_point) {
			return is_numeric(code_point) || in_range(code_point, 'A', 'F') || code_point == '_';
		}
		
		bool is_tab(CodePointT code_point) {
			return code_point == '\t';
		}
		
		bool is_space(CodePointT code_point) {
			return code_point == ' ';
		}
		
		bool is_whitespace(CodePointT i) {
			return is_tab(i) || is_space(i);
		}
		
		bool is_whitespace_or_newline(CodePointT code_point) {
			return code_point == ' ' || code_point == '\t' || code_point == '\r' || code_point == '\n';
		}
		
		bool is_newline(CodePointT code_point) {
			return code_point == '\r' || code_point == '\n';
		}
		
		bool is_not_newline(CodePointT code_point) {
			return !is_newline(code_point);
		}

		// Credit for bits of this code go to Markus Kuhn.
		struct CodePointInterval {
			CodePointT first;
			CodePointT last;
		};

		// Auxiliary function for binary search in interval table.
		static std::size_t binary_search(CodePointT ucs, const struct CodePointInterval *table, std::size_t max) {
			std::size_t min = 0, mid;

			if (ucs < table[0].first || ucs > table[max].last)
				return 0;
			while (max >= min) {
				mid = (min + max) / 2;
				if (ucs > table[mid].last)
					min = mid + 1;
				else if (ucs < table[mid].first)
					max = mid - 1;
				else
					return 1;
			}

			return 0;
		}


		/* The following two functions define the column width of an ISO 10646
		 * character as follows:
		 *
		 *    - The null character (U+0000) has a column width of 0.
		 *
		 *    - Other C0/C1 control characters and DEL will lead to a return
		 *      value of -1.
		 *
		 *    - Non-spacing and enclosing combining characters (general
		 *      category code Mn or Me in the Unicode database) have a
		 *      column width of 0.
		 *
		 *    - SOFT HYPHEN (U+00AD) has a column width of 1.
		 *
		 *    - Other format characters (general category code Cf in the Unicode
		 *      database) and ZERO WIDTH SPACE (U+200B) have a column width of 0.
		 *
		 *    - Hangul Jamo medial vowels and final consonants (U+1160-U+11FF)
		 *      have a column width of 0.
		 *
		 *    - Spacing characters in the East Asian Wide (W) or East Asian
		 *      Full-width (F) category as defined in Unicode Technical
		 *      Report #11 have a column width of 2.
		 *
		 *    - All remaining characters (including all printable
		 *      ISO 8859-1 and WGL4 characters, Unicode control characters,
		 *      etc.) have a column width of 1.
		 *
		 * This implementation assumes that CodePointT characters are encoded
		 * in ISO 10646.
		 */

		std::size_t fixed_width(CodePointT ucs)
		{
			/* sorted list of non-overlapping intervals of non-spacing characters */
			/* generated by "uniset +cat=Me +cat=Mn +cat=Cf -00AD +1160-11FF +200B c" */
			static const struct CodePointInterval combining[] = {
				{ 0x0300, 0x036F }, { 0x0483, 0x0486 }, { 0x0488, 0x0489 },
				{ 0x0591, 0x05BD }, { 0x05BF, 0x05BF }, { 0x05C1, 0x05C2 },
				{ 0x05C4, 0x05C5 }, { 0x05C7, 0x05C7 }, { 0x0600, 0x0603 },
				{ 0x0610, 0x0615 }, { 0x064B, 0x065E }, { 0x0670, 0x0670 },
				{ 0x06D6, 0x06E4 }, { 0x06E7, 0x06E8 }, { 0x06EA, 0x06ED },
				{ 0x070F, 0x070F }, { 0x0711, 0x0711 }, { 0x0730, 0x074A },
				{ 0x07A6, 0x07B0 }, { 0x07EB, 0x07F3 }, { 0x0901, 0x0902 },
				{ 0x093C, 0x093C }, { 0x0941, 0x0948 }, { 0x094D, 0x094D },
				{ 0x0951, 0x0954 }, { 0x0962, 0x0963 }, { 0x0981, 0x0981 },
				{ 0x09BC, 0x09BC }, { 0x09C1, 0x09C4 }, { 0x09CD, 0x09CD },
				{ 0x09E2, 0x09E3 }, { 0x0A01, 0x0A02 }, { 0x0A3C, 0x0A3C },
				{ 0x0A41, 0x0A42 }, { 0x0A47, 0x0A48 }, { 0x0A4B, 0x0A4D },
				{ 0x0A70, 0x0A71 }, { 0x0A81, 0x0A82 }, { 0x0ABC, 0x0ABC },
				{ 0x0AC1, 0x0AC5 }, { 0x0AC7, 0x0AC8 }, { 0x0ACD, 0x0ACD },
				{ 0x0AE2, 0x0AE3 }, { 0x0B01, 0x0B01 }, { 0x0B3C, 0x0B3C },
				{ 0x0B3F, 0x0B3F }, { 0x0B41, 0x0B43 }, { 0x0B4D, 0x0B4D },
				{ 0x0B56, 0x0B56 }, { 0x0B82, 0x0B82 }, { 0x0BC0, 0x0BC0 },
				{ 0x0BCD, 0x0BCD }, { 0x0C3E, 0x0C40 }, { 0x0C46, 0x0C48 },
				{ 0x0C4A, 0x0C4D }, { 0x0C55, 0x0C56 }, { 0x0CBC, 0x0CBC },
				{ 0x0CBF, 0x0CBF }, { 0x0CC6, 0x0CC6 }, { 0x0CCC, 0x0CCD },
				{ 0x0CE2, 0x0CE3 }, { 0x0D41, 0x0D43 }, { 0x0D4D, 0x0D4D },
				{ 0x0DCA, 0x0DCA }, { 0x0DD2, 0x0DD4 }, { 0x0DD6, 0x0DD6 },
				{ 0x0E31, 0x0E31 }, { 0x0E34, 0x0E3A }, { 0x0E47, 0x0E4E },
				{ 0x0EB1, 0x0EB1 }, { 0x0EB4, 0x0EB9 }, { 0x0EBB, 0x0EBC },
				{ 0x0EC8, 0x0ECD }, { 0x0F18, 0x0F19 }, { 0x0F35, 0x0F35 },
				{ 0x0F37, 0x0F37 }, { 0x0F39, 0x0F39 }, { 0x0F71, 0x0F7E },
				{ 0x0F80, 0x0F84 }, { 0x0F86, 0x0F87 }, { 0x0F90, 0x0F97 },
				{ 0x0F99, 0x0FBC }, { 0x0FC6, 0x0FC6 }, { 0x102D, 0x1030 },
				{ 0x1032, 0x1032 }, { 0x1036, 0x1037 }, { 0x1039, 0x1039 },
				{ 0x1058, 0x1059 }, { 0x1160, 0x11FF }, { 0x135F, 0x135F },
				{ 0x1712, 0x1714 }, { 0x1732, 0x1734 }, { 0x1752, 0x1753 },
				{ 0x1772, 0x1773 }, { 0x17B4, 0x17B5 }, { 0x17B7, 0x17BD },
				{ 0x17C6, 0x17C6 }, { 0x17C9, 0x17D3 }, { 0x17DD, 0x17DD },
				{ 0x180B, 0x180D }, { 0x18A9, 0x18A9 }, { 0x1920, 0x1922 },
				{ 0x1927, 0x1928 }, { 0x1932, 0x1932 }, { 0x1939, 0x193B },
				{ 0x1A17, 0x1A18 }, { 0x1B00, 0x1B03 }, { 0x1B34, 0x1B34 },
				{ 0x1B36, 0x1B3A }, { 0x1B3C, 0x1B3C }, { 0x1B42, 0x1B42 },
				{ 0x1B6B, 0x1B73 }, { 0x1DC0, 0x1DCA }, { 0x1DFE, 0x1DFF },
				{ 0x200B, 0x200F }, { 0x202A, 0x202E }, { 0x2060, 0x2063 },
				{ 0x206A, 0x206F }, { 0x20D0, 0x20EF }, { 0x302A, 0x302F },
				{ 0x3099, 0x309A }, { 0xA806, 0xA806 }, { 0xA80B, 0xA80B },
				{ 0xA825, 0xA826 }, { 0xFB1E, 0xFB1E }, { 0xFE00, 0xFE0F },
				{ 0xFE20, 0xFE23 }, { 0xFEFF, 0xFEFF }, { 0xFFF9, 0xFFFB },
				{ 0x10A01, 0x10A03 }, { 0x10A05, 0x10A06 }, { 0x10A0C, 0x10A0F },
				{ 0x10A38, 0x10A3A }, { 0x10A3F, 0x10A3F }, { 0x1D167, 0x1D169 },
				{ 0x1D173, 0x1D182 }, { 0x1D185, 0x1D18B }, { 0x1D1AA, 0x1D1AD },
				{ 0x1D242, 0x1D244 }, { 0xE0001, 0xE0001 }, { 0xE0020, 0xE007F },
				{ 0xE0100, 0xE01EF }
			};
			static const std::size_t combining_length = sizeof(combining) / sizeof(struct CodePointInterval) - 1;

			// Test for 8-bit control characters:
			if (ucs == 0)
				return 0;
			if (ucs < 32 || (ucs >= 0x7f && ucs < 0xa0))
				return -1;

			// Binary search in table of non-spacing characters:
			if (binary_search(ucs, combining, combining_length))
				return 0;

			// If we arrive here, ucs is not a combining or C0/C1 control character
			return 1 +
				(ucs >= 0x1100 &&
					(ucs <= 0x115f ||                    /* Hangul Jamo init. consonants */
						ucs == 0x2329 || ucs == 0x232a ||
							(ucs >= 0x2e80 && ucs <= 0xa4cf &&
							 ucs != 0x303f) ||                  /* CJK ... Yi */
							(ucs >= 0xac00 && ucs <= 0xd7a3) || /* Hangul Syllables */
							(ucs >= 0xf900 && ucs <= 0xfaff) || /* CJK Compatibility Ideographs */
							(ucs >= 0xfe10 && ucs <= 0xfe19) || /* Vertical forms */
							(ucs >= 0xfe30 && ucs <= 0xfe6f) || /* CJK Compatibility Forms */
							(ucs >= 0xff00 && ucs <= 0xff60) || /* Fullwidth Forms */
							(ucs >= 0xffe0 && ucs <= 0xffe6) ||
							(ucs >= 0x20000 && ucs <= 0x2fffd) ||
							(ucs >= 0x30000 && ucs <= 0x3fffd)));
		}

		std::size_t fixed_width(const std::string & value) {
			auto current = value.begin(), end = value.end();

			std::size_t width = 0;

			while (current != end) {
				auto code_point = Unicode::next(current, end);

				width += Unicode::fixed_width(code_point);
			}

			return width;
		}

		std::size_t sequence_length(unsigned char starting_character) {
			uint8_t lead = utf8::internal::mask8(starting_character);

			if (lead < 0x80)
				return 1;
			else if ((lead >> 5) == 0x6)
				return 2;
			else if ((lead >> 4) == 0xe)
				return 3;
			else if ((lead >> 3) == 0x1e)
				return 4;
			else
				return 0;
		}
	}
}
