# brick.jpg Loading - Diagnostic & Fix Guide

## Problem Analysis

The brick.jpg file EXISTS (7845 bytes, valid JPEG) but `stbi_load()` is failing to load it. This indicates a **PATH RESOLUTION ISSUE**, not a file corruption issue.

### Root Cause - Working Directory Problem
When your application runs, the working directory might not be where you expect:

```
Executable might run from:  C:\Users\USER\Desktop\Graphics-Project\build\Debug\
But textures are in:        C:\Users\USER\Desktop\Graphics-Project\textures\
Expected relative path:     ../../textures/brick.jpg
Actual relative path tried: textures/brick.jpg
```

## Solution 1: Update Project Settings (Visual Studio)

### Step 1: Set Working Directory
1. Open your Visual Studio project
2. Go to: **Project ? Properties**
3. Navigate to: **Debugging**
4. Set **Working Directory** to your project root:
   ```
   C:\Users\USER\Desktop\Graphics-Project\
   ```
5. Click **Apply** and **OK**

### Step 2: Rebuild
```
Build ? Clean Solution
Build ? Rebuild Solution
Run the application (F5)
```

## Solution 2: Hardcode Absolute Path (Temporary Testing)

Edit `src/main.cpp` in the `loadSceneTextures()` function, temporarily replace:

```cpp
texBrick = loadTexture(findResource("textures/brick.jpg").c_str());
```

With:

```cpp
texBrick = loadTexture("C:\\Users\\USER\\Desktop\\Graphics-Project\\textures\\brick.jpg");
```

If this works, it confirms the issue is path resolution (not the file or stbi_load).

## Solution 3: Fix Path Search Order

The `findResource()` function now includes better debugging. When you run the app, you'll see:

```
[CWD] C:\path\to\current\working\directory
? Found: textures/brick.jpg -> correct/path/to/file
```

Or:

```
[CWD] C:\path\to\wrong\directory
? Resource NOT found: textures/brick.jpg
  Checked paths:
    - textures/brick.jpg
    - ../textures/brick.jpg
    - ../../textures/brick.jpg
    (and 15 more paths)
```

## Solution 4: Add Output Directory Copy

Visual Studio can automatically copy textures to the output directory:

1. Go to: **Project ? Properties ? Build Events**
2. In **Post-Build Event**, add:
   ```
   xcopy "$(ProjectDir)textures" "$(OutDir)textures" /E /Y
   ```
3. This copies all textures to the executable directory

## Solution 5: Use Relative Path from Executable

If executable is in `Graphics-Project\build\Debug\`, modify path:

```cpp
texBrick = loadTexture(findResource("../../../textures/brick.jpg").c_str());
```

## Quick Test - Add Debug Output

Add this to `loadSceneTextures()` right before loading brick:

```cpp
cout << "\n=== BRICK TEXTURE DEBUG ===" << endl;
cout << "Attempting: " << findResource("textures/brick.jpg") << endl;
cout << "File exists: " << (fileExists(findResource("textures/brick.jpg")) ? "YES" : "NO") << endl;
cout << "==========================\n" << endl;
```

## Expected Console Output After Fix

```
--- LOADING TEXTURES ---
[CWD] C:\Users\USER\Desktop\Graphics-Project
  -> Loading brick texture...
  [Attempting] textures/brick.jpg
  [? OK] Loaded: textures/brick.jpg (512x512, 3ch)

  -> Loading concrete texture...
  [Attempting] textures/map.jpg
  [? OK] Loaded: textures/map.jpg (512x512, 3ch)
```

## What NOT to Do

? Don't modify the brick.jpg file - it's fine  
? Don't try to re-encode the JPEG - not necessary  
? Don't change stbi_load settings - defaults are correct  
? Don't move textures folder - leave it in Graphics-Project/textures/

## Debugging Steps

1. **Add print statements** showing current working directory
2. **Verify directory exists** using Windows Explorer
3. **Check file permissions** - make sure user can read the file
4. **Try absolute path** to confirm file itself is loadable
5. **Check build output** - ensure textures weren't excluded from build

## Next Steps

1. ? Run application with updated `resource_utils.h`
2. ? Check console output for `[CWD]` message
3. ? Look for `? Found:` or `? Resource NOT found:`
4. ? If not found, try Solution 1 (set working directory)
5. ? Rebuild and test again

---

**Key Point**: The problem is NOT with brick.jpg itself - it's that the file path resolution isn't finding it from the current working directory. Setting the working directory in Visual Studio project properties should fix this immediately.
