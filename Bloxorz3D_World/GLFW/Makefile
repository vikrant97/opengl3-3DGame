all: sample2D

sample2D: Sample_GL3_2D.cpp glad.c
	g++ -O2 -o sample2D Sample_GL3_2D.cpp glad.c -fpermissive -lpthread -lGL -lglfw -ldl -lmpg123 -lao 

clean:
	rm sample2D
