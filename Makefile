all:
	g++ main.cpp quickcg.cpp -o raycaster -lSDL -O2

run:
	./raycaster
