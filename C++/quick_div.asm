.code ; Start code section of the file

quick_div proc ; Start quick_div function. Input is coming in via ecx register
	shr ecx, 1 ; Shift ecx register by 1. This basically divides by 2 without keeping sign
	mov eax, ecx ; Move ecx register into eax register

	ret ; Return eax register
quick_div endp ; End quick_div function

END ; End file
