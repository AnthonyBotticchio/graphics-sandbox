# --- Dependencies ---

find_package(SDL3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

# --- Subdirs ---

add_subdirectory(engine)
add_subdirectory(sample)
