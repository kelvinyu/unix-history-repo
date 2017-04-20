# $FreeBSD$

atf_test_case simple
atf_test_case unified
atf_test_case header
atf_test_case header_ns
atf_test_case ifdef

simple_body()
{
	atf_check -o file:$(atf_get_srcdir)/simple.out -s eq:1 \
		diff "$(atf_get_srcdir)/input1.in" "$(atf_get_srcdir)/input2.in"

	atf_check -o file:$(atf_get_srcdir)/simple_e.out -s eq:1 \
		diff -e "$(atf_get_srcdir)/input1.in" "$(atf_get_srcdir)/input2.in"

	atf_check -o file:$(atf_get_srcdir)/simple_u.out -s eq:1 \
		diff -u -L input1 -L input2 "$(atf_get_srcdir)/input1.in" "$(atf_get_srcdir)/input2.in"

	atf_check -o file:$(atf_get_srcdir)/simple_n.out -s eq:1 \
		diff -n "$(atf_get_srcdir)/input1.in" "$(atf_get_srcdir)/input2.in"

	atf_check -o inline:"Files $(atf_get_srcdir)/input1.in and $(atf_get_srcdir)/input2.in differ\n" -s eq:1 \
		diff -q "$(atf_get_srcdir)/input1.in" "$(atf_get_srcdir)/input2.in"

	atf_check \
		diff -q "$(atf_get_srcdir)/input1.in" "$(atf_get_srcdir)/input1.in"

	atf_check -o file:$(atf_get_srcdir)/simple_i.out -s eq:1 \
		diff -i "$(atf_get_srcdir)/input_c1.in" "$(atf_get_srcdir)/input_c2.in"

	atf_check -o file:$(atf_get_srcdir)/simple_w.out -s eq:1 \
		diff -w "$(atf_get_srcdir)/input_c1.in" "$(atf_get_srcdir)/input_c2.in"

	atf_check -o file:$(atf_get_srcdir)/simple_b.out -s eq:1 \
		diff -b "$(atf_get_srcdir)/input_c1.in" "$(atf_get_srcdir)/input_c2.in"

	atf_check -o file:$(atf_get_srcdir)/simple_p.out -s eq:1 \
		diff --label input_c1.in --label input_c2.in -p "$(atf_get_srcdir)/input_c1.in" "$(atf_get_srcdir)/input_c2.in"
}

unified_body()
{
	atf_check -o file:$(atf_get_srcdir)/unified_p.out -s eq:1 \
		diff -up -L input_c1.in -L input_c2.in  "$(atf_get_srcdir)/input_c1.in" "$(atf_get_srcdir)/input_c2.in"
	atf_check -o file:$(atf_get_srcdir)/unified_c9999.out -s eq:1 \
		diff -u -c9999 -L input_c1.in -L input_c2.in "$(atf_get_srcdir)/input_c1.in" "$(atf_get_srcdir)/input_c2.in"
	atf_check -o file:$(atf_get_srcdir)/unified_9999.out -s eq:1 \
		diff -u9999 -L input_c1.in -L input_c2.in "$(atf_get_srcdir)/input_c1.in" "$(atf_get_srcdir)/input_c2.in"
}

header_body()
{
	export TZ=UTC
	: > empty
	echo hello > hello
	touch -d 2015-04-03T01:02:03 empty
	touch -d 2016-12-22T11:22:33 hello
	atf_check -o "file:$(atf_get_srcdir)/header.out" -s eq:1 \
		diff -u empty hello
}

header_ns_body()
{
	export TZ=UTC
	: > empty
	echo hello > hello
	touch -d 2015-04-03T01:02:03.123456789 empty
	touch -d 2016-12-22T11:22:33.987654321 hello
	atf_check -o "file:$(atf_get_srcdir)/header_ns.out" -s eq:1 \
		diff -u empty hello
}

ifdef_body()
{
	atf_check -o file:$(atf_get_srcdir)/ifdef.out -s eq:1 \
		diff -D PLOP "$(atf_get_srcdir)/input_c1.in" \
		"$(atf_get_srcdir)/input_c2.in"
}

atf_init_test_cases()
{
	atf_add_test_case simple
	atf_add_test_case unified
	atf_add_test_case header
	atf_add_test_case header_ns
	atf_add_test_case ifdef
}
