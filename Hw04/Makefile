weather: main_weather.c parser_json.c
	gcc -g -Wall -Wextra -Wpedantic -std=c11 main_weather.c parser_json.c -l curl -l json-c -o weather

all: weather

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions weather

.PHONY: clean
