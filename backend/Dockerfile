# Stage 1: Build Environment
FROM ubuntu:24.04 AS build

# Install build tools (build-essential includes GCC and g++)
RUN apt-get update \
    && apt-get install -y --no-install-recommends build-essential cmake \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF \
    && cmake --build build --config Release --target graph_engine

# Stage 2: Minimal Runtime Environment
FROM ubuntu:24.04

# Install the runtime C++ libraries required to run the engine
RUN apt-get update \
    && apt-get install -y --no-install-recommends libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY --from=build /app/build/graph_engine /app/graph_engine

EXPOSE 8080
CMD ["./graph_engine"]