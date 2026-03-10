CXX      = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude
LDFLAGS  =

SRC_DIR  = src
SRCS     = $(wildcard $(SRC_DIR)/*.cpp)
OBJS     = $(SRCS:.cpp=.o)

TARGET   = raytracer1c

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean