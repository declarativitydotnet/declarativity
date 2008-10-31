all: bin/stasis/jni/libstasisjni.so
	@echo; echo "Be sure to add this to java's command line:" ;echo
	@echo "  -Djava.library.path=`pwd`/bin/stasis/jni" ; echo

#STASIS_DIR=/home/sears/stasis4
STASIS_DIR=/Users/peteralvaro/code/stasis/stasis-read-only
JAVA_DIR=/System/Library/Frameworks/JavaVM.framework/Versions/1.6/Home/

bin/stasis/jni/Stasis.class : src/stasis/jni/Stasis.java
	cd src;	javac -d ../bin \
	    stasis/jni/Stasis.java

src/stasis/jni/stasis_jni_Stasis.h : bin/stasis/jni/Stasis.class
	javah -classpath bin -jni \
	    -d src/stasis/jni/ \
	    stasis.jni.Stasis

bin/stasis/jni/libstasisjni.so: src/stasis/jni/stasis_jni_Stasis.c \
				src/stasis/jni/stasis_jni_Stasis.h
	gcc  -fPIC \
	    -I $(JAVA_DIR)/include \
            -I $(STASIS_DIR) \
	    $(STASIS_DIR)/build/src/stasis/libstasis.dylib \
               src/stasis/jni/stasis_jni_Stasis.c \
	    -dylib -o bin/stasis/jni/libstasisjni.dylib

.PHONY: clean

clean :
	rm bin/stasis/jni/Stasis.class \
	   src/stasis/jni/stasis_jni_Stasis.h \
	   bin/stasis/jni/libstasisjni.dylib

.PHONY: test
test : all
	java -cp bin -Djava.library.path=bin/stasis/jni stasis.jni.Stasis
