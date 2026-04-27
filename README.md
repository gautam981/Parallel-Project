# ThreadX: Parallel DAG Task Scheduler (Hinglish Explanation)

Yeh document aapke project **"Parallel DAG-Based Task Scheduling with Dynamic Load Balancing using Work-Stealing"** ka ek simple aur detailed explanation hai. Isko padhne ke baad aapko poora concept aur code ka flow asani se samajh aa jayega.

---

## 1. Basic Concepts (Aasan Bhasha Mein)

### A. DAG (Directed Acyclic Graph) Kya Hota Hai?
Imagine karo aapko ek ghar banana hai. 
- Pura ghar banane ka process chote-chote **tasks** mein divided hai (e.g., eent lagana, cement daalna, paint karna).
- Kuch tasks ek dusre pe **dependent** hote hain. Jaise jab tak deewar (wall) khadi nahi hogi, aap paint nahi kar sakte.
- Is dependency ko represent karne ke liye hum **DAG (Directed Acyclic Graph)** use karte hain. 
  - **Directed:** Matlab task A ke baad B hoga (A -> B).
  - **Acyclic:** Matlab koi loop nahi hona chahiye (A -> B -> A galat hai, warna kaam kabhi khatam nahi hoga).

### B. Parallel Scheduling Kya Hai?
Agar aapke paas 4 mazdoor (threads) hain, toh aap unhe ek-ek karke kaam karne ke bajaye, jo tasks independent hain (jaise 4 alag deewaro par paint karna), wo ek sath (parallel) karwa sakte ho. Isse kaam jaldi khatam hota hai (makespan reduce hota hai).

### C. Work-Stealing Aur Dynamic Load Balancing Kya Hai?
Socho aapke paas 2 threads (mazdoor) hain, T1 aur T2. Dono ko 10-10 tasks diye gaye.
- Agar T1 jaldi free ho gaya aur T2 abhi bhi kaam kar raha hai, toh T1 khali baithne ke bajaye T2 ke bachhe hue tasks mein se kuch **"steal" (chura)** kar lega.
- Isko bolte hain **Work-Stealing**. Isse "Dynamic Load Balancing" maintain hoti hai, matlab koi bhi thread khali (idle) nahi baithta aur processors ka 100% sahi use hota hai.

---

## 2. Code Structure & Files Explanation

Project mein total 4 main C++ files hain. Aaiye dekhte hain har file aur function kya karta hai:

### 1. `dag.h` (The Structure)
Yeh file tasks aur unki dependencies (graph) define karti hai.
- **`Task` Struct:** Har task ka ek `id` hota hai. Isme ek `dependency_count` (atomic integer) hota hai jo batata hai ki is task ko shuru hone se pehle kitne aur tasks ka khatam hona zaroori hai. Iske andar `dependents` ki list hoti hai (yani wo tasks jo iske khatam hone ka wait kar rahe hain).
- **`DAG` Class:** Isme saare tasks ki ek list hoti hai. `addDependency(u, v)` function batata hai ki task `v` tabhi chalega jab task `u` khatam ho jayega.

### 2. `thread_pool.h` (The Engine)
Yeh system ka engine hai jahan threads aur unke queues manage hote hain.
- **Queue System:** Har thread ke paas apna ek personal `std::deque` (Double-Ended Queue) hota hai.
- **`pushTask()`**: Jab koi task ready ho jata hai, toh use kisi ek thread ke queue mein daal diya jata hai. Iske baad ek `cv.notify_one()` signal bheja jata hai taaki agar koi thread so raha ho, toh wo uth jaye.
- **`popTask()` (LIFO):** Thread apne khud ke queue se tasks nikalta hai. Yeh hamesha queue ke **Peeche (back)** se nikalta hai (LIFO - Last In First Out). Iska fayda yeh hai ki CPU cache fast kaam karta hai kyunki recent memory use hoti hai.
- **`stealTask()` (FIFO):** Agar ek thread ka apna queue khali ho gaya hai, toh wo doosre threads ke queue se tasks churata hai. Lekin chori hamesha **Aage (front)** se hoti hai (FIFO - First In First Out). Aisa isliye kyunki aage wale tasks sabse purane aur bade hote hain, jisse chori karne wale ko jyada kaam milta hai aur wo baar-baar chori karne wapis nahi aata.

### 3. `scheduler.h` / `scheduler.cpp` (The Brain)
Yeh file decide karti hai ki tasks kab aur kaise execute honge.
- **`Scheduler::run()`**: Yeh function execution shuru karta hai. Yeh sabse pehle un tasks ko dhoondta hai jinki `dependency_count` 0 hai (yani jo shuru hone ke liye bilkul ready hain) aur unhe threads ko assign kar deta hai. Fir yeh sabhi threads ko parallel mein start kar deta hai.
- **`Scheduler::worker()`**: Yeh har thread ki loop hai jo lagatar chalti rehti hai. 
  - Thread pehle apne queue se task nikalne ki koshish karta hai (`popTask`).
  - Agar queue khali hai, toh wo churane ki koshish karta hai (`stealTask`).
  - Agar chori karne ke liye bhi kuch nahi milta, toh CPU power bachane ke liye thread `wait_for` (Condition Variable) use karke **so jata hai (sleep)**, taaki PC hang na ho aur processor free rahe.
- **`Scheduler::executeTask()`**: Yeh function ek dummy loop (1 Million iterations) chalata hai, jo ek asli computational task ko mimic (copy) karta hai. Is loop ko pura karne mein thoda time lagta hai jisse hume parallel vs sequential ka farq pata chalta hai.

### 4. `main.cpp` (The Benchmark & UI)
Yeh driver program hai jo poore experiment ko chalata hai.
- **`generateTreeDAG()`**: Yeh function ek complete Binary Tree jaisa graph banata hai (1 node ke 2 bachhe). Binary tree isliye banaya hai kyunki isme bohot saare branches hote hain jo ek sath parallel mein run kiye ja sakte hain.
- **`runSequential()`**: Yeh base test hai jo saare tasks ko bina kisi thread ke (ek-ek karke) chalata hai aur time note karta hai.
- **Parallel Loop (`for (int t : thread_counts)`)**: Yeh system ko alag-alag threads (1, 2, 4, 8, 16) par chalata hai. Har baar ek naya DAG create kiya jata hai taaki pichli baar ke zero huye dependency counters naye test ko kharab na karein. Uske baad yeh "Speedup" calculate karta hai (Sequential Time / Parallel Time).
- **Console Output**: Jo sundar table output aap terminal mein dekhte hain, wo yehi file print karti hai, aur saara data `results.csv` mein save karti hai.

### 5. Extra Tools
- **`plot_results.py`**: Yeh Python script `results.csv` ko read karti hai aur `matplotlib` use karke uske data ka ek khoobsurat graph (`performance_graph.png`) bana deti hai.
- **`Makefile`**: Yeh C++ code ko easily compile aur run karne ke liye Linux script hai. (`make clean` kachra saaf karta hai, `make run` code build karke chalata hai).

---

## 3. Quick Summary (Viva / Presentation ke liye)

Agar koi pooche aapka project kya karta hai, toh bas yeh 3 points boliye:
1. "Humne ek C++ parallel scheduler banaya hai jo **Directed Acyclic Graphs (DAG)** ke dependencies ko samajhta hai."
2. "Humne **Work-Stealing Deque** implement kiya hai. Jab ek thread idle (khali) hota hai, toh wo LIFO (local) aur FIFO (stealing) techniques use karke dusre threads se kaam chura leta hai, isse processor ki maximum efficiency milti hai."
3. "Is scheduling ko implement karne ke baad humne apne test mein almost **5.4x ka speedup** achieve kiya jab humne apne threads badhaye, jo humare benchmark graph mein cleanly dikhta hai."
