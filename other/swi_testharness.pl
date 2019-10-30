:- module(swi_testharness, [test_file/1]).

strip_prompt_and_dot(Line, NoPromptDot) :-
    sub_string(Line, _, Length, After, "?- "),
    sub_string(Line, Length, After, _, NoPrompt),
    (
     sub_string(NoPrompt, Before, _, _, ")."),!,
     Before1 is Before + 1
     ;
     sub_string(NoPrompt, Before1, _, _, ".")
     ),
    sub_string(NoPrompt, 0, Before1, _, NoPromptDot).

strip_comment(Line, NoComment) :-
    sub_string(Line, Before, _Length, _After, "%"),
    sub_string(Line, 0, Before, _, NoComment).

strip_expect_comment(Line, NoPrompt) :-
    sub_string(Line, _Before, Length, After, "% Expect:"),
    sub_string(Line, Length, After, _, NoPrompt).

is_expect_end_line(Line) :-
    sub_string(Line, _, _, _, "% Expect: end").

check_file(FileName) :-
    exists_file(FileName), !.
check_file(FileName) :-
    message("Error: File '~s' not found", FileName),
    fail.

test_file(FileName) :-
    check_file(FileName),
    load_files([FileName]),
    open(FileName, read, Stream),
    read_lines(Stream, TestCases),
    fuse_tests(TestCases, Fused),
    check_tests(Fused).

check_tests([]).
check_tests([TS|TSs]) :-
    term_string(T, TS),
    write("Running:"),
    write(TS),
    write("\n"),
    call(T),
    check_tests(TSs).

read_lines(Stream, Tests) :-
    read_line_to_string(Stream, String),
    read_line(String, Stream, Tests).

read_line(end_of_file, _, []) :- !.
read_line(Line, Stream, [[TestRun | TestChecks] | Tests]) :-
    strip_prompt_and_dot(Line, TestRun),!,
    read_expect_lines(Stream, TestChecks),
    read_lines(Stream, Tests).
read_line(_, Stream, Tests) :-
    read_lines(Stream, Tests).

read_expect_lines(Stream, TestChecks) :-
    read_line_to_string(Stream, String),
    read_expect_line(String, Stream, TestChecks).

read_expect_line(end_of_file, _, []) :- !.
read_expect_line(Line, _, []) :-
    is_expect_end_line(Line), !.
read_expect_line(Line, Stream, [TestCheck|TestChecks]) :-
    strip_expect_comment(Line, TestCheck),!,
    read_expect_lines(Stream, TestChecks).
read_expect_line(_, Stream, TestChecks) :-
    read_expect_lines(Stream, TestChecks).

fuse_tests([], []).
fuse_tests([[TestRun|TestChecks]|TestPoints], [F|Fs]) :-
    concat_checks(TestChecks, FusedChecks),
    string_concat(TestRun, ", ", TestRunComma),
    string_concat(TestRunComma, FusedChecks, F),
    fuse_tests(TestPoints, Fs).

concat_checks([], "").
concat_checks([S], S) :- !.
concat_checks([S|Ss], Ds) :-
    concat_checks(Ss, Bs),
    string_concat(S, ", fail; ", Cs),
    string_concat(Cs, Bs, Ds).
