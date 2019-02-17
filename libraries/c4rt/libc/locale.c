#include <locale.h>

static struct lconv locale_info = {
	// TODO: is any of this right?
	.decimal_point = ".",
	.thousands_sep = ",",
	.grouping      = "",
	.int_curr_symbol = "USD,",
	.currency_symbol = "$",
	.mon_decimal_point = ".",
	.mon_grouping = ",",
	.positive_sign = "+",
	.negative_sign = "-",
	.int_frac_digits = 100,
	.frac_digits = 100,
	.p_cs_precedes = 1,
	.p_sep_by_space = 0,
	.n_cs_precedes = 1,
	.n_sep_by_space = 0,

	.p_sign_posn = 4,
	.p_sign_posn = 4,
};

struct lconv *localeconv(void) {
	return &locale_info;
}
