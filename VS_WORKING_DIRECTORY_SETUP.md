# Visual Studio Configuration Guide - Set Working Directory

## Quick Fix for brick.jpg Loading Issue

The issue is that Visual Studio runs your executable from the **build output directory**, not the **project root directory**.

---

## Step-by-Step Guide

### Step 1: Open Project Properties

In Visual Studio:
1. **Solution Explorer** (right panel)
2. **Right-click on your PROJECT** (not the solution)
3. Click **Properties**

> ?? Make sure you right-click the PROJECT, not the solution!

### Step 2: Navigate to Debugging Settings

In the Properties window:
1. **Left panel** ? Find **Debugging** (or search for it)
2. Click **Debugging**

### Step 3: Set Working Directory

In the **Debugging** settings:
1. Find the **Working Directory** field
2. Clear any existing value
3. Enter the full path to your project root:
   ```
   C:\Users\USER\Desktop\Graphics-Project
   ```

> Make sure to replace `USER` with your actual Windows username!

### Step 4: Apply Settings

1. Click **Apply** button
2. Click **OK** button

---

## Alternative Method (Visual Studio 2019+)

If "Debugging" isn't showing:

1. **Project** (top menu) ? **Properties**
2. **Configuration Properties** (left panel) ? **Debugging**
3. Set **Working Directory**
4. Click **OK**

---

## Verify It Worked

1. **Build ? Clean Solution**
2. **Build ? Rebuild Solution**
3. **Debug ? Start Debugging** (or press **F5**)
4. **Check Console Output**:
   - Look for: `[Working Directory] C:\Users\USER\Desktop\Graphics-Project`
   - Should show: `? Found: textures/brick.jpg`

---

## Expected Results

### Console Output (Before Fix)
```
--- LOADING TEXTURES ---
[Working Directory] C:\Users\USER\Desktop\Graphics-Project\build\Debug
  -> Loading brick texture...
  [Attempting] textures/brick.jpg
  ? Resource NOT found: textures/brick.jpg
  -> Brick texture not found, generating procedural brick texture...
```

### Console Output (After Fix)
```
--- LOADING TEXTURES ---
[Working Directory] C:\Users\USER\Desktop\Graphics-Project
  -> Loading brick texture...
  [Attempting] textures/brick.jpg
  ? Found: textures/brick.jpg -> textures/brick.jpg
  [? OK] Loaded: textures/brick.jpg (512x512, 3ch)
```

---

## Screenshots Reference

### Finding Project Properties
```
Solution Explorer (Right Panel)
  ?? Your Solution
      ?? YOUR_PROJECT_NAME ? RIGHT-CLICK HERE
          ?? Select "Properties"
```

### Finding Debugging Settings
```
Properties Window
Left Panel:
  - Common Properties
  - Configuration Properties
      ?? General
      ?? VC++ Directories
      ?? Debugging ? CLICK HERE
      ?? ...
```

### Setting Working Directory
```
Debugging Properties
  Working Directory: [________________] ? Type path here
  
  C:\Users\USER\Desktop\Graphics-Project
```

---

## Troubleshooting

### "Properties not showing Debugging option"
- **Solution**: Make sure you right-clicked the PROJECT, not the solution
- Right-click on project name ? Properties (not Solution's Properties)

### "Working Directory field is greyed out"
- **Solution**: Make sure configuration is set to **Debug** (dropdown at top)
- Check the **Configuration** dropdown shows **Debug**

### "Still not working after setting path"
- **Solution**: Clean and rebuild
- **Build ? Clean Solution**
- **Build ? Rebuild Solution**
- **Debug ? Start Debugging** (F5)

### "textures folder not found at that location"
- **Solution**: Verify the path exists
- Open Windows Explorer
- Navigate to: `C:\Users\USER\Desktop\Graphics-Project`
- Check that `textures` folder exists there

---

## Additional Notes

### Multiple Configurations

If you have multiple build configurations (Debug, Release, etc.):
- Set working directory for **EACH configuration**
- Or set for **All Configurations**

To set for all:
1. **Configuration** dropdown (top of Properties window)
2. Select **All Configurations**
3. Set Working Directory
4. Click **OK**

### Command Line Alternative

You can also run from command line with working directory:
```cmd
cd C:\Users\USER\Desktop\Graphics-Project\build\Debug
cd ..\..\
"build\Debug\YourApp.exe"
```

---

## Quick Checklist

- [ ] Opened Project Properties (not Solution Properties)
- [ ] Found Debugging section
- [ ] Set Working Directory to: `C:\Users\USER\Desktop\Graphics-Project`
- [ ] Clicked Apply and OK
- [ ] Did Clean Solution
- [ ] Did Rebuild Solution
- [ ] Running with F5 or Debug menu
- [ ] Checked console for Working Directory message
- [ ] Verified ? Found messages for textures

---

**Once this is done, brick.jpg should load successfully!**
