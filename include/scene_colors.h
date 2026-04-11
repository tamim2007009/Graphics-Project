#ifndef SCENE_COLORS_H
#define SCENE_COLORS_H

#include <glm/glm.hpp>

// Colors
//  Exterior
const glm::vec3 C_GRASS(0.22f, 0.55f, 0.18f);
const glm::vec3 C_GRASS2(0.18f, 0.48f, 0.15f);
const glm::vec3 C_ROAD(0.25f, 0.25f, 0.27f);
const glm::vec3 C_SIDEWALK(0.68f, 0.66f, 0.62f);
const glm::vec3 C_ROAD_YEL(0.95f, 0.82f, 0.12f);
const glm::vec3 C_ROAD_WHT(0.92f, 0.92f, 0.90f);
const glm::vec3 C_PARKING(0.42f, 0.42f, 0.44f);
const glm::vec3 C_PARK_LINE(0.95f, 0.85f, 0.15f);
const glm::vec3 C_EXT(0.80f, 0.78f, 0.74f); // exterior wall
const glm::vec3 C_GLASS(0.58f, 0.80f, 0.93f);
const glm::vec3 C_AWNING(0.38f, 0.38f, 0.42f);
const glm::vec3 C_TRUNK(0.42f, 0.30f, 0.16f);
const glm::vec3 C_CROWN(0.22f, 0.58f, 0.20f);
const glm::vec3 C_LAMP(0.32f, 0.32f, 0.34f);
const glm::vec3 C_TIRE(0.12f, 0.12f, 0.12f);
const glm::vec3 C_BARRIER_R(0.90f, 0.15f, 0.10f);

// Interior
const glm::vec3 C_FLOOR(0.82f, 0.78f, 0.72f); // mall floor
const glm::vec3 C_CEILING(0.93f, 0.91f, 0.89f);
const glm::vec3 C_CORR(0.85f, 0.83f, 0.78f); // corridor wall

// Fashion shop - PINK walls (from reference)
const glm::vec3 C_FASH_WALL(0.95f, 0.68f, 0.72f);
const glm::vec3 C_FASH_FLOOR(0.78f, 0.65f, 0.55f);
const glm::vec3 C_RACK(0.70f, 0.70f, 0.72f); // clothing rack metal
const glm::vec3 C_MIRROR(0.75f, 0.88f, 0.95f);
const glm::vec3 C_MANNEQUIN(0.85f, 0.78f, 0.65f);
const glm::vec3 C_CASH_DESK(0.60f, 0.45f, 0.30f);

// Tech shop - BLUE-GREY walls (from reference)
const glm::vec3 C_TECH_WALL(0.68f, 0.75f, 0.85f);
const glm::vec3 C_TECH_FLOOR(0.65f, 0.65f, 0.68f);
const glm::vec3 C_DISP_TABLE(0.72f, 0.60f, 0.45f); // display table wood
const glm::vec3 C_DISP_BASE(0.50f, 0.50f, 0.52f);  // table base grey
const glm::vec3 C_PHONE(0.15f, 0.15f, 0.18f);      // phone/device
const glm::vec3 C_SCREEN(0.25f, 0.62f, 0.45f);     // screen green-ish
const glm::vec3 C_LAPTOP(0.55f, 0.55f, 0.58f);

// Gems shop
const glm::vec3 C_GEM_WALL(0.92f, 0.85f, 0.65f);
const glm::vec3 C_GEM_CASE(0.70f, 0.68f, 0.65f);

// Book shop
const glm::vec3 C_BOOK_WALL(0.72f, 0.58f, 0.42f);
const glm::vec3 C_BOOKSHELF(0.50f, 0.35f, 0.22f);

// Food court (from reference - colorful stalls)
const glm::vec3 C_FOOD_FLOOR(0.72f, 0.70f, 0.66f);
const glm::vec3 C_STALL_RED(0.88f, 0.22f, 0.15f);
const glm::vec3 C_STALL_YEL(0.92f, 0.82f, 0.18f);
const glm::vec3 C_STALL_GRN(0.20f, 0.72f, 0.30f);
const glm::vec3 C_STALL_ORG(0.95f, 0.55f, 0.12f);
const glm::vec3 C_STALL_PUR(0.60f, 0.30f, 0.70f);
const glm::vec3 C_TABLE(0.62f, 0.48f, 0.30f);
const glm::vec3 C_CHAIR(0.55f, 0.40f, 0.25f);
const glm::vec3 C_COUNTER(0.65f, 0.60f, 0.55f);

// Prayer room (from reference - calm beige/cream)
const glm::vec3 C_PRAY_WALL(0.90f, 0.87f, 0.80f);
const glm::vec3 C_PRAY_FLOOR(0.78f, 0.68f, 0.55f);
const glm::vec3 C_PRAY_MAT(0.55f, 0.72f, 0.58f);
const glm::vec3 C_PRAY_LAMP(0.92f, 0.88f, 0.75f);
const glm::vec3 C_PRAY_SHELF(0.65f, 0.55f, 0.42f);

// Washroom / Staircase
const glm::vec3 C_WASH(0.88f, 0.88f, 0.86f);
const glm::vec3 C_STAIR(0.58f, 0.56f, 0.52f);
const glm::vec3 C_PARAPET(0.72f, 0.70f, 0.66f);

// Factory / Industrial colors
const glm::vec3 C_FACTORY_FLOOR(0.35f, 0.35f, 0.36f);
const glm::vec3 C_CONVEYOR(0.12f, 0.12f, 0.14f);
const glm::vec3 C_BOX(0.76f, 0.60f, 0.36f);
const glm::vec3 C_METAL(0.55f, 0.56f, 0.58f);
const glm::vec3 C_MACHINE(0.2f, 0.2f, 0.22f);
const glm::vec3 C_ROLLER(0.45f, 0.45f, 0.48f);
const glm::vec3 C_CYAN_ACCENT(0.0f, 0.8f, 1.0f);
const glm::vec3 C_DARK_METAL(0.25f, 0.25f, 0.28f);

#endif
