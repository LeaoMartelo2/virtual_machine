toggle_verbose 0
jmp .start

# arg_a = message, arg_b = fd
func_print:
    ld $arg_a, $arg_c
    strlen_r $arg_c, $arg_d
    mov write_syscall, $arg_a

    syscall

ret

# $11 = readinput(stdin, 1024)
func_readinput:
    ld $heap, $11

    
    mov read_syscall, $arg_a
    mov stdin, $arg_b
    ld $heap, $arg_c
    mov 1024, $arg_d
    syscall

    mov 0, $9
    ldxo $11, $9, $15
    # store the first character of the read string to $15


ret

func_prompt:
    mov stdout, $arg_b
    call .func_print

ret

func_to_html:
    call .func_readinput
    ld $11, $arg_a
    ld $10, $arg_b
    call .func_print

    mov @newline, $arg_a
    ld $10, $arg_b
    call .func_print

ret
    
func_make_page_header:
    mov @page_header, $arg_a
    ld $10, $arg_b
    call .func_print

    mov @newline, $arg_a
    ld $10, $arg_b
    call .func_print

ret

func_start_body:
    mov @page_start_body, $arg_a
    ld $10, $arg_b
    call .func_print
    
ret

func_end_body:
    mov @page_end_body, $arg_a
    ld $10, $arg_b
    call .func_print
ret

func_make_title:

    mov @page_h1_tag_start, $arg_a
    ld $10, $arg_b
    call .func_print

    mov @enter_title, $arg_a
    call .func_prompt
    call .func_to_html

    mov @page_h1_tag_end, $arg_a
    ld $10, $arg_b
    call .func_print

ret

func_make_subtitle:
    mov @page_h2_tag_start, $arg_a
    ld $10, $arg_b
    call .func_print


    mov @enter_subtitle, $arg_a
    call .func_prompt
    call .func_to_html

    mov @page_h2_tag_end, $arg_a
    ld $10, $arg_b
    call .func_print

ret

func_make_line:
    call .func_to_html
ret

func_make_linebreak:
    mov @page_br_tag, $arg_a
    ld $10, $arg_b
    call .func_print
ret


start:
# open file
mov open_syscall, $arg_a
mov @filename, $arg_b
mov O_RDWR, $0
mov O_CREAT, $1
mov O_TRUNC, $2
or $0, $1
or $0, $2
ld $0, $arg_c
mov perm_0644, $arg_d
syscall
ld $arg_a, $10          # $10 = FD

call .func_make_page_header

call .func_start_body

    call .func_make_title
    call .func_make_subtitle


    mov @enter_maintext, $arg_a
    mov stdout, $arg_b
    call .func_print

    page_readloop:

        call .func_make_line
        mov 126, $5
        cmp, $5, $15
        #print_char $15
        #line_br
        #print_int $15
        call .func_make_linebreak
        jne .page_readloop
    

call .func_end_body
mov @page_endpage, $arg_a
ld $10, $arg_b
call .func_print

# close FD
mov close_syscall, $arg_a
ld $10, $arg_b
syscall

halt

.data
    filename: "index.html"
    newline: 10 0

    empty: ""

    enter_title: "Page Title: "
    enter_subtitle: "Subtitle: "
enter_maintext: 
"
Enter a line to the page (Enter a '~' to finish the page): 
"

page_header: 
"
<!DOCTYPE html> 
<head> 
<title>Generated Page</title>
</head>
"

page_start_body:
"
<body>
"

page_end_body: 
"
</body>
"

page_endpage: 
"
</html>
"

page_h1_tag_start: 
"
<h1>
"
page_h1_tag_end:   
"
</h1>
"

page_h2_tag_start:
"
<h2>
"

page_h2_tag_end:
"
</h2>
"

page_br_tag:
"
<br>
"
    
