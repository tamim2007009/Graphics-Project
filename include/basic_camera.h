//
//  basic_camera.h
//  First-Person Camera with Rodrigues Rotation
//
//  Modified for Parking Garage Project
//

#ifndef basic_camera_h
#define basic_camera_h

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <cmath>

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
};

class BasicCamera {
public:

    glm::vec3 eye;
    glm::vec3 lookAt;
    glm::vec3 V;
    float Yaw, Pitch;
    float Zoom;
    float MovementSpeed;
    float MouseSensitivity;

    BasicCamera(float eyeX = 0.0, float eyeY = 1.7, float eyeZ = 3.0,
        float lookAtX = 0.0, float lookAtY = 1.7, float lookAtZ = 0.0,
        glm::vec3 viewUpVector = glm::vec3(0.0f, 1.0f, 0.0f))
    {
        eye = glm::vec3(eyeX, eyeY, eyeZ);
        lookAt = glm::vec3(lookAtX, lookAtY, lookAtZ);
        V = viewUpVector;

        Yaw = -90.0f;   // facing -Z initially
        Pitch = 0.0f;
        Zoom = 45.0f;
        MovementSpeed = 4.0f;
        MouseSensitivity = 0.1f;

        updateVectors();
    }

    // ---- Rodrigues rotation: rotate vector v around unit axis k by angle theta (radians) ----
    // v_rot = v*cos(theta) + (k x v)*sin(theta) + k*(k.v)*(1 - cos(theta))
    static glm::vec3 rodriguesRotate(glm::vec3 v, glm::vec3 k, float thetaRad)
    {
        float cosT = cos(thetaRad);
        float sinT = sin(thetaRad);
        glm::vec3 cross_kv = glm::vec3(
            k.y * v.z - k.z * v.y,
            k.z * v.x - k.x * v.z,
            k.x * v.y - k.y * v.x
        );
        float dot_kv = k.x * v.x + k.y * v.y + k.z * v.z;
        return v * cosT + cross_kv * sinT + k * dot_kv * (1.0f - cosT);
    }

    // Build view matrix manually using the camera vectors (no glm::lookAt or glm::rotate)
    glm::mat4 createViewMatrix()
    {
        // We build the lookAt matrix manually:
        // n = normalize(eye - target)   (camera looks along -n)
        // u = normalize(V x n)
        // v = u x n                     (not cross with V, but with n)

        glm::vec3 target = eye + front;  // look target

        glm::vec3 n = glm::normalize(eye - target);  // backward direction
        // cross product V x n => u (right)
        glm::vec3 u = glm::normalize(glm::vec3(
            V.y * n.z - V.z * n.y,
            V.z * n.x - V.x * n.z,
            V.x * n.y - V.y * n.x
        ));
        // cross product n x u => v (up, recalculated)
        glm::vec3 vv = glm::vec3(
            n.y * u.z - n.z * u.y,
            n.z * u.x - n.x * u.z,
            n.x * u.y - n.y * u.x
        );

        // View matrix = rotation * translation
        // rotation part transposes (u, vv, n) into rows
        // translation part = -rotation * eye
        float tx = -(u.x * eye.x + u.y * eye.y + u.z * eye.z);
        float ty = -(vv.x * eye.x + vv.y * eye.y + vv.z * eye.z);
        float tz = -(n.x * eye.x + n.y * eye.y + n.z * eye.z);

        glm::mat4 viewMat(1.0f);
        viewMat[0][0] = u.x;  viewMat[1][0] = u.y;  viewMat[2][0] = u.z;  viewMat[3][0] = tx;
        viewMat[0][1] = vv.x; viewMat[1][1] = vv.y; viewMat[2][1] = vv.z; viewMat[3][1] = ty;
        viewMat[0][2] = n.x;  viewMat[1][2] = n.y;  viewMat[2][2] = n.z;  viewMat[3][2] = tz;
        viewMat[0][3] = 0.0f; viewMat[1][3] = 0.0f; viewMat[2][3] = 0.0f; viewMat[3][3] = 1.0f;

        return viewMat;
    }

    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= yoffset;
        if (Zoom < 1.0f) Zoom = 1.0f;
        if (Zoom > 45.0f) Zoom = 45.0f;
    }

    void ProcessMouseMovement(float xoffset, float yoffset)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        if (Pitch > 89.0f) Pitch = 89.0f;
        if (Pitch < -89.0f) Pitch = -89.0f;

        updateVectors();
    }

    void move(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;

        // Movement in XZ plane only (pedestrian — Y stays fixed)
        glm::vec3 flatFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
        glm::vec3 flatRight = glm::normalize(glm::vec3(right.x, 0.0f, right.z));

        if (direction == FORWARD) {
            eye += flatFront * velocity;
        }
        if (direction == BACKWARD) {
            eye -= flatFront * velocity;
        }
        if (direction == LEFT) {
            eye -= flatRight * velocity;
        }
        if (direction == RIGHT) {
            eye += flatRight * velocity;
        }
    }

    glm::vec3 getDirection()
    {
        return front;
    }

    // Public accessors to camera vectors
    glm::vec3 front;
    glm::vec3 right;
    glm::vec3 up;

private:
    void updateVectors()
    {
        // Compute front direction from Yaw and Pitch using trig (Rodrigues concept)
        // Start with a base forward vector (0,0,-1) and rotate it
        // Equivalent direct formula:
        float yawRad = Yaw * 3.14159265f / 180.0f;
        float pitchRad = Pitch * 3.14159265f / 180.0f;

        front.x = cos(yawRad) * cos(pitchRad);
        front.y = sin(pitchRad);
        front.z = sin(yawRad) * cos(pitchRad);
        front = glm::normalize(front);

        // Alternatively derive via Rodrigues:
        // 1) Start with base vector (1,0,0)
        // 2) Rotate around Y by Yaw -> gives us front in XZ plane
        // 3) Then rotate around the resulting right vector by Pitch
        // The direct trig formula above IS the result of Rodrigues for these axis rotations.

        // Right = front x worldUp (using manual cross product, no glm::cross dependency needed
        // but glm::normalize is just a sqrt division, acceptable)
        glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
        right = glm::normalize(glm::vec3(
            front.y * worldUp.z - front.z * worldUp.y,
            front.z * worldUp.x - front.x * worldUp.z,
            front.x * worldUp.y - front.y * worldUp.x
        ));

        // Up = right x front
        up = glm::normalize(glm::vec3(
            right.y * front.z - right.z * front.y,
            right.z * front.x - right.x * front.z,
            right.x * front.y - right.y * front.x
        ));

        // Keep lookAt in sync
        lookAt = eye + front;
    }
};

#endif /* basic_camera_h */
