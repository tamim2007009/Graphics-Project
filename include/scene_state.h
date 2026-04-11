#ifndef SCENE_STATE_H
#define SCENE_STATE_H

#include <glm/glm.hpp>
#include <vector>

// Toggle flags
inline bool texturesEnabled = true;
inline bool ambientEnabled = true;
inline bool pointLightsEnabled = true;
inline bool dirLightEnabled = true;
inline bool spotLightsEnabled = true;
inline bool noLightCondition = false;
inline int shadingMode = 0; // 0 = Phong, 1 = Gouraud

// Day/Night cycle
inline float timeOfDay = 10.0f;  // 0-24 hours, start at 10 AM
inline float dayNightSpeed = 0.0f; // hours per second (0 = paused, press N to cycle)
inline bool dayNightCycleActive = false;

// Elevator state machine
enum ElevatorState { ELEV_STOPPED_BOTTOM, ELEV_DOOR_CLOSING_UP, ELEV_MOVING_UP, ELEV_STOPPED_TOP, ELEV_DOOR_CLOSING_DOWN, ELEV_MOVING_DOWN, ELEV_DOOR_OPENING };
inline ElevatorState elevatorState = ELEV_STOPPED_BOTTOM;
inline float elevatorY = 0.0f;
inline float elevatorDoorOffset = 0.0f; // 0 = closed, 1 = open
inline float elevatorTimer = 0.0f;

// Escalator
inline float escalatorOffset = 0.0f;
inline int escalatorDir = 1; // 1=up, -1=down, 0=paused

// Boom barrier
inline float barrierAngle = 0.0f; // 0=closed, 90=open
inline bool barrierOpen = false;

// Wheel rotation for cars
inline float wheelRotation = 0.0f;

// Cursor capture toggle
inline bool cursorCaptured = true;

// Entrance glass door
inline bool entranceDoorOpen = false;
inline float entranceDoorOffset = 0.0f; // 0=closed, 1=open

// Texture IDs
inline unsigned int texCubeVAO = 0;
inline unsigned int texFloor = 0, texFashion = 0, texTech = 0, texGems = 0, texFood = 0, texTreeLeaf = 0, texTreeBark = 0, texGrass = 0, texIndustryFloor = 0;
inline unsigned int texBrick = 0, texConcrete = 0, texMetal = 0, texWood = 0, texRoof = 0, texAsphalt = 0;

// Assignment Features Exhibition State
inline bool exShowcase = true;       // Key '6' - Show/Hide Exhibition Stand
inline bool exBlendMode = false;     // Key '7' - false = Simple, true = Blended
inline bool exVertexShade = false;   // Key '8' - false = Fragment, true = Vertex
inline int exObjectType = 0;         // Key '9' - 0 = Sphere, 1 = Cone

// Factory animation state
inline float conveyorOffset = 0.0f;
inline float globalTime = 0.0f;
inline float conveyorSpeed = 2.0f;  // Controllable conveyor speed (units/sec)
inline bool conveyorRunning = true; // Toggle conveyor on/off
inline bool packagingRunning = true; // Toggle packaging system
inline float packingSpeed = 1.0f;  // Controllable packing speed

// Palletizer animation state (pick -> move -> place -> return)
inline float palletizerCycle = 0.0f;
inline int palletizerStackCount = 0;
inline int palletizerCurrentSource = -1;

// Box tracking for realistic handling
struct Box {
    float position;  // Linear position along path
    glm::vec3 color; // Current color
    bool active;     // Is this box in the system?
    float creationTime; // When was this box created?
    float rotation;  // Box rotation for realistic tumbling
    float vx, vz;    // Velocity components for physics
    unsigned int textureID; // Texture ID for the applied texture
};
inline const int MAX_BOXES = 50;
inline Box boxes[MAX_BOXES];
inline float boxCreationRate = 4.0f;  // Synchronized process: 1 box every 4.0s (matches machine cycles)
inline float lastBoxCreationTime = 0.0f;
inline int gatheredBoxes = 0; // Finished packed boxes gathered in shipping area

// Factory Lift System
inline float factoryLiftY = 0.0f;
inline int factoryLiftState = 0; // 0=gathering, 1=up, 2=hold, 3=down
inline float factoryLiftWait = 0.0f;

// Machine states for realistic kinematics
struct MachineState {
    float pressStroke;       // 0 to 1: press head position
    float pressVelocity;     // Velocity of press head
    bool pressExtending;     // Direction of press motion
    float weldArmAngle1;     // Joint 1 rotation
    float weldArmAngle2;     // Joint 2 rotation
    float weldTorchGlow;     // Glow intensity from welding
    float robotGripperOpen;  // 0 to 1: gripper opening
};
inline MachineState machineState = {0.5f, 0.0f, false, 0.0f, 0.0f, 0.0f, 0.5f};

// Forward declaration of the updateAnimations logic
inline void updateAnimations(float deltaTime) {
    // STAMPING PRESS: Precise hydraulic motion with realistic force profile
    float pressMotionTime = 4.0f;
    float pressCyclePhase = fmod(globalTime, pressMotionTime) / pressMotionTime;
    
    if (pressCyclePhase < 0.1f) {
        float approach = pressCyclePhase / 0.1f;
        float easeIn = approach * approach * (3.0f - 2.0f * approach);
        machineState.pressStroke = 0.5f - easeIn * 0.48f;
        machineState.pressVelocity = -easeIn * 8.5f;
    } else if (pressCyclePhase < 0.50f) {
        float dwellTime = (pressCyclePhase - 0.1f) / 0.4f;
        float vibrationFreq = 9.5f + dwellTime * 1.5f;
        float vibrationAmplitude = 0.008f * (1.0f - dwellTime * 0.3f);
        float resonanceFreq = 15.0f;
        float resonanceAmplitude = 0.003f * sin(dwellTime * 3.14159f);
        machineState.pressStroke = 0.02f + sin(globalTime * vibrationFreq) * vibrationAmplitude + 
                                  sin(globalTime * resonanceFreq) * resonanceAmplitude;
        machineState.pressVelocity = (cosf(globalTime * vibrationFreq) * vibrationAmplitude + 
                                     cosf(globalTime * resonanceFreq) * resonanceAmplitude) * 0.5f;
    } else if (pressCyclePhase < 0.85f) {
        float returnPhase = (pressCyclePhase - 0.50f) / 0.35f;
        float easeOut = 1.0f - (1.0f - returnPhase) * (1.0f - returnPhase) * (1.0f - returnPhase);
        machineState.pressStroke = 0.02f + easeOut * 0.48f;
        machineState.pressVelocity = easeOut * 11.0f;
    } else {
        machineState.pressStroke = 0.5f;
        machineState.pressVelocity = 0.0f;
    }
    
    float weldCycleTime = 4.0f;
    float weldCyclePhase = fmod(globalTime, weldCycleTime) / weldCycleTime;
    machineState.weldArmAngle1 = fmod(globalTime * 18.0f, 360.0f);
    
    if (weldCyclePhase < 0.15f) {
        float approachTime = weldCyclePhase / 0.15f;
        float accelProfile = approachTime * approachTime * (3.0f - 2.0f * approachTime);
        machineState.weldArmAngle2 = 25.0f + accelProfile * 40.0f;
    } else if (weldCyclePhase < 0.68f) {
        float weldTime = (weldCyclePhase - 0.15f) / 0.53f;
        float seamMotion = sin(weldTime * 3.14159f * 2.0f) * 3.2f;
        float microCorrection = sin(globalTime * 4.5f) * 1.5f;
        machineState.weldArmAngle2 = 65.0f + seamMotion + microCorrection;
    } else if (weldCyclePhase < 0.88f) {
        float retractTime = (weldCyclePhase - 0.68f) / 0.20f;
        float decelerationProfile = 1.0f - (1.0f - retractTime) * (1.0f - retractTime) * (1.0f - retractTime);
        machineState.weldArmAngle2 = 65.0f - decelerationProfile * 40.0f;
    } else {
        machineState.weldArmAngle2 = 25.0f;
    }
    
    machineState.weldArmAngle2 += sin(globalTime * 0.65f) * 6.5f;
    
    if (weldCyclePhase >= 0.15f && weldCyclePhase < 0.68f) {
        float weldIntensity = sin((weldCyclePhase - 0.15f) / 0.53f * 3.14159f) * 0.5f + 0.5f;
        float arcCurrent = sin(globalTime * 14.5f) * 0.35f + 0.65f;
        float heatBuildUp = (weldCyclePhase - 0.15f) / 0.53f;
        float totalGlow = 0.5f + (weldIntensity * 0.35f) + (arcCurrent * 0.25f) + (heatBuildUp * 0.1f);
        machineState.weldTorchGlow = glm::clamp(totalGlow, 0.3f, 1.0f);
    } else {
        float fadePhase = (weldCyclePhase >= 0.68f) ? (weldCyclePhase - 0.68f) : (weldCyclePhase + (1.0f - 0.68f));
        machineState.weldTorchGlow = glm::max(0.1f - fadePhase * 0.1f, 0.05f);
    }
    
    if (weldCyclePhase < 0.12f) {
        float openPhase = weldCyclePhase / 0.12f;
        float easeOut = 1.0f - (1.0f - openPhase) * (1.0f - openPhase);
        machineState.robotGripperOpen = 0.85f - easeOut * 0.5f;
    } else if (weldCyclePhase < 0.18f) {
        float closePhase = (weldCyclePhase - 0.12f) / 0.06f;
        float easeIn = closePhase * closePhase * (3.0f - 2.0f * closePhase);
        machineState.robotGripperOpen = 0.35f - easeIn * 0.28f;
    } else if (weldCyclePhase < 0.68f) {
        float holdPhase = (weldCyclePhase - 0.18f) / 0.50f;
        float weldIntensity = sin(holdPhase * 3.14159f * 2.0f) * 0.5f + 0.5f;
        float graspForce = 0.07f + (weldIntensity * 0.08f);
        float weldVibration = sin(globalTime * 8.0f) * 0.002f;
        machineState.robotGripperOpen = graspForce + weldVibration;
    } else if (weldCyclePhase < 0.82f) {
        float releasePhase = (weldCyclePhase - 0.68f) / 0.14f;
        float easeOut = 1.0f - (1.0f - releasePhase) * (1.0f - releasePhase);
        machineState.robotGripperOpen = 0.07f + easeOut * 0.78f;
    } else {
        machineState.robotGripperOpen = 0.85f;
    }

    if (conveyorRunning) {
        conveyorOffset += deltaTime * conveyorSpeed;
    }
    globalTime += deltaTime;

    lastBoxCreationTime += deltaTime;
    if (lastBoxCreationTime >= boxCreationRate && conveyorRunning) {
        for (int i = 0; i < MAX_BOXES; i++) {
            if (!boxes[i].active) {
                boxes[i].active = true;
                boxes[i].position = -25.0f;
                boxes[i].color = glm::vec3(0.8f, 0.3f, 0.15f);
                boxes[i].creationTime = globalTime;
                boxes[i].textureID = 0;
                lastBoxCreationTime = 0.0f;
                break;
            }
        }
    }

    for (int i = 0; i < MAX_BOXES; i++) {
        if (boxes[i].active) {
            if (conveyorRunning) {
                float step = deltaTime * conveyorSpeed * 2.0f;
                bool takesLiftBranch = (i % 3 == 0);
                
                if (!packagingRunning && boxes[i].position >= 8.0f && !takesLiftBranch)
                    step = 0.0f;
                
                if (takesLiftBranch && boxes[i].position >= 20.0f) {
                    bool blocked = false;
                    
                    if (factoryLiftState == 0) {
                        if (boxes[i].position >= 28.5f) blocked = true;
                    } else if (factoryLiftState == 1 || factoryLiftState == 3) {
                        if (boxes[i].position >= 26.2f && boxes[i].position <= 29.5f) blocked = true;
                    } else if (factoryLiftState == 2) {
                        if (boxes[i].position >= 26.2f && boxes[i].position <= 27.0f) blocked = true;
                    }
                    
                    if (!blocked) {
                        for(int j=0; j<MAX_BOXES; j++) {
                            if(i!=j && boxes[j].active && (j%3==0) && boxes[j].position > boxes[i].position && boxes[j].position < 40.0f && (boxes[j].position - boxes[i].position) < 0.65f) {
                                blocked = true;
                                break;
                            }
                        }
                    }
                    if (blocked) step = 0.0f;
                }

                boxes[i].position += step;
            }
            
            float pathDistance = boxes[i].position + 25.0f;
            boxes[i].rotation = fmod(pathDistance * 20.0f, 360.0f);

            if (boxes[i].position < -20.0f) {
                boxes[i].color = glm::vec3(0.8f, 0.35f, 0.15f);
                boxes[i].textureID = 0;
            } 
            else if (boxes[i].position < -9.0f) {
                float stageFactor = (boxes[i].position + 20.0f) / 11.0f;
                boxes[i].color = glm::mix(
                    glm::vec3(0.8f, 0.35f, 0.15f), 
                    glm::vec3(0.8f, 0.1f, 0.1f), 
                    stageFactor
                );
                boxes[i].textureID = 0;
            } 
            else if (boxes[i].position < -7.0f) {
                glm::vec3 brightColors[] = {
                     glm::vec3(0.1f, 0.6f, 1.0f),
                     glm::vec3(1.0f, 0.2f, 0.5f),
                     glm::vec3(1.0f, 0.8f, 0.0f),
                     glm::vec3(0.1f, 0.9f, 0.2f),
                     glm::vec3(1.0f, 0.5f, 0.0f),
                     glm::vec3(0.7f, 0.2f, 1.0f)
                };
                boxes[i].color = brightColors[i % 6];
                boxes[i].textureID = 0;
            }
            else if (boxes[i].position < 6.0f) {
                if (boxes[i].textureID == 0 && boxes[i].position > -6.0f) {
                    unsigned int textures[] = {texTech, texFashion, texGems, texFood, texBrick, texWood};
                    boxes[i].textureID = textures[i % 6];
                }
                float weldEffect = machineState.weldTorchGlow * 0.15f;
                boxes[i].color += glm::vec3(weldEffect, weldEffect, weldEffect * 0.5f);
            }
            else if (boxes[i].position < 10.0f) {
                if (!(i % 3 == 0)) {
                    boxes[i].textureID = 0;
                    float packFactor = glm::clamp((boxes[i].position - 6.0f) / 4.0f, 0.0f, 1.0f);
                    boxes[i].color = glm::mix(boxes[i].color, glm::vec3(0.92f, 0.92f, 0.90f), packFactor);
                }
            }
            else if (boxes[i].position < 40.0f) {
                if (!(i % 3 == 0)) {
                    boxes[i].textureID = 0;
                }
            }
            else {
                boxes[i].active = false;
            }
        }
    }

    int boxesOnLift = 0;
    for (int i = 0; i < MAX_BOXES; i++) {
        if (boxes[i].active && (i % 3 == 0) && boxes[i].position >= 27.0f && boxes[i].position <= 28.6f) {
            boxesOnLift++;
        }
    }
    
    if (factoryLiftState == 0) {
        if (boxesOnLift >= 3) {
            factoryLiftState = 1;
        }
    } else if (factoryLiftState == 1) {
        factoryLiftY += deltaTime * 2.0f;
        if (factoryLiftY >= 3.0f) {
            factoryLiftY = 3.0f;
            factoryLiftState = 2;
            factoryLiftWait = 0.0f;
        }
    } else if (factoryLiftState == 2) {
        bool liftClear = true;
        for(int i=0; i<MAX_BOXES; i++) {
            if (boxes[i].active && (i % 3 == 0) && boxes[i].position >= 27.0f && boxes[i].position <= 29.5f) {
                liftClear = false;
                break;
            }
        }
        if (liftClear) {
            factoryLiftState = 3;
        }
    } else if (factoryLiftState == 3) {
        factoryLiftY -= deltaTime * 2.0f;
        if (factoryLiftY <= 0.0f) {
            factoryLiftY = 0.0f;
            factoryLiftState = 0;
        }
    }

    if (packagingRunning) {
        palletizerCycle += deltaTime * packingSpeed;
        if (palletizerCycle >= 1.0f) palletizerCycle -= 1.0f;
    }

    if (packagingRunning && palletizerCurrentSource < 0 && palletizerCycle < 0.18f) {
        for (int i = 0; i < MAX_BOXES; ++i) {
            if (boxes[i].active && boxes[i].position > 8.0f && boxes[i].position < 12.0f) {
                palletizerCurrentSource = i;
                break;
            }
        }
    }

    if (packagingRunning && palletizerCurrentSource >= 0 && palletizerCycle > 0.72f && palletizerCycle < 0.80f) {
        if (boxes[palletizerCurrentSource].active) {
            boxes[palletizerCurrentSource].active = false;
            palletizerStackCount++;
            
            if (palletizerStackCount >= 16) {
                palletizerStackCount -= 16;
                if (gatheredBoxes < 72) gatheredBoxes += 16;
            }
        }
        palletizerCurrentSource = -1;
    }

    escalatorOffset += escalatorDir * deltaTime * 2.0f;
    if (escalatorOffset > 10.0f) escalatorOffset -= 10.0f;
    if (escalatorOffset < 0.0f) escalatorOffset += 10.0f;

    float targetAngle = barrierOpen ? 90.0f : 0.0f;
    if (barrierAngle < targetAngle) barrierAngle = glm::min(barrierAngle + deltaTime * 90.0f, targetAngle);
    if (barrierAngle > targetAngle) barrierAngle = glm::max(barrierAngle - deltaTime * 90.0f, targetAngle);

    float doorTarget = entranceDoorOpen ? 1.0f : 0.0f;
    if (entranceDoorOffset < doorTarget) entranceDoorOffset = glm::min(entranceDoorOffset + deltaTime * 1.2f, doorTarget);
    if (entranceDoorOffset > doorTarget) entranceDoorOffset = glm::max(entranceDoorOffset - deltaTime * 1.2f, doorTarget);
}

#endif
