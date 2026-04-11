# QUICK START: 3-Step Brick.jpg Fix

## Step 1: Open Project Properties (30 seconds)

```
Visual Studio Window
    ?
Left Panel: Solution Explorer
    ?
Find Your Project (not the Solution)
    ?
RIGHT-CLICK on PROJECT NAME
    ?
Select "Properties"
```

**Result**: Properties window opens

---

## Step 2: Set Working Directory (1 minute)

```
Properties Window
    ?
Left Panel: Configuration Properties
    ?
Click "Debugging"
    ?
Find "Working Directory" field
    ?
CLEAR the field
    ?
TYPE: C:\Users\USER\Desktop\Graphics-Project
```

**Make sure to replace `USER` with your actual Windows username!**

---

## Step 3: Apply and Rebuild (1 minute)

```
Click "Apply" button
    ?
Click "OK" button
    ?
Visual Studio Menu: Build
    ?
Select "Clean Solution"
    ?
Select "Rebuild Solution"
    ?
Wait for build to complete
```

**Expected**: "Build Succeeded"

---

## Step 4: Run and Verify (30 seconds)

```
Press F5 (or Debug ? Start Debugging)
    ?
Application starts
    ?
Look at Console Output Window
    ?
Find line: [Working Directory] C:\Users\USER\Desktop\Graphics-Project
    ?
Find line: ? Found: textures/brick.jpg
    ?
Find line: [? OK] Loaded: textures/brick.jpg (512x512, 3ch)
```

**If you see those messages: ? SUCCESS!**

---

## Total Time: ~3-4 minutes

---

## Visual Verification

Once running, to see brick texture:

1. Walk around in the factory scene
2. Look at walls or floors
3. Press **T** key to toggle textures on/off
4. You should see brick pattern appear/disappear

---

## If It Still Doesn't Work

Check:

1. **Working directory path is correct?**
   - Should be: `C:\Users\USER\Desktop\Graphics-Project`
   - NOT: `C:\Users\USER\Desktop\Graphics-Project\build\Debug`

2. **File exists at that location?**
   - Open Windows Explorer
   - Navigate to: `C:\Users\USER\Desktop\Graphics-Project\textures`
   - Should see: `brick.jpg` file

3. **Rebuild was done?**
   - Build ? Clean Solution
   - Build ? Rebuild Solution
   - NOT just Build

4. **Console shows correct working directory?**
   - Look for: `[Working Directory] C:\Users\USER\Desktop\Graphics-Project`
   - If wrong, working directory setting didn't apply

---

## Done! ??

Your brick.jpg should now load successfully!

For more details, see: `README_BRICK_FIX.md` or `VS_WORKING_DIRECTORY_SETUP.md`
