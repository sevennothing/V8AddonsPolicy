cmd_Release/obj.target/seller_grade.node := flock ./Release/linker.lock g++ -shared -pthread -rdynamic -m64  -Wl,-soname=seller_grade.node -o Release/obj.target/seller_grade.node -Wl,--start-group Release/obj.target/seller_grade/sellerActiveGrade.o -Wl,--end-group 
