CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2
TARGET = anbernic_favorites_tool
SRCS = anbernic_favorites_tool.cc
SCRIPTS = Shuffle\ favorites.sh Sort\ favorites.sh
ZIPFILE = anbernic_favorites_tool.zip

all: $(TARGET) zip

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

zip: $(TARGET)
	mkdir -p favorites_tool
	zip $(ZIPFILE) $(SCRIPTS)
	cp $(TARGET) favorites_tool/
	zip -r $(ZIPFILE) favorites_tool
	rm -r favorites_tool

clean:
	rm -f $(TARGET) $(ZIPFILE)

.PHONY: all zip clean
