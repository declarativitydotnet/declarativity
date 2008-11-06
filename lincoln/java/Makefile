all: checkvars bin/stasis/jni/libstasisjni.so
	@echo; echo "Be sure to add this to java's command line:" ;echo
	@echo "  -Djava.library.path=`pwd`/bin/stasis/jni" ; echo

bin/stasis/jni/Stasis.class : src/stasis/jni/Stasis.java
	cd src;	javac -d ../bin \
	    stasis/jni/Stasis.java

src/stasis/jni/stasis_jni_Stasis.h : bin/stasis/jni/Stasis.class
	javah -classpath bin -jni \
	    -d src/stasis/jni/ \
	    stasis.jni.Stasis

bin/stasis/jni/libstasisjni.so: src/stasis/jni/stasis_jni_Stasis.c \
				src/stasis/jni/stasis_jni_Stasis.h
	if [ "$(shell uname)" = "Linux" ] ; then \
	  gcc -shared -fPIC \
	      -I $(JAVA_DIR)/include \
	      -I $(STASIS_DIR) \
	      -L $(STASIS_DIR)/build/src/libdfa \
	      -L $(STASIS_DIR)/build/src/stasis \
	         src/stasis/jni/stasis_jni_Stasis.c \
	      -l stasis \
	      -l rw \
	      -o bin/stasis/jni/libstasisjni.so ; \
	else \
	  gcc -fPIC \
	      -I $(JAVA_DIR)/include \
	      -I $(STASIS_DIR) \
	       $(STASIS_DIR)/build/src/stasis/libstasis.dylib \
	       src/stasis/jni/stasis_jni_Stasis.c \
	       -dylib -o bin/stasis/jni/libstasisjni.dylib ; \
	fi

.PHONY: clean

clean :
	rm -f bin/stasis/jni/Stasis.class \
	      src/stasis/jni/stasis_jni_Stasis.h \
	      bin/stasis/jni/libstasisjni.so

.PHONY: checkvars
checkvars :
	@if [[ ! -d "$(STASIS_DIR)" ]] ; then  echo 'Define $$STASIS_DIR to continue!'; exit 1; fi
	@if [[ ! -d "$(JAVA_DIR)" ]] ; then  echo 'Define $$JAVA_DIR to continue!'; exit 1; fi


.PHONY: test
test : checkvars all
	LD_LIBRARY_PATH=$(STASIS_DIR)/build/src/stasis:$(STASIS_DIR)/build/src/libdfa \
	java -cp bin -Djava.library.path=/home/sears/workspace/jol/bin/stasis/jni stasis.jni.Stasis
