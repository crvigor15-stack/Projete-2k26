import { initializeApp } from "https://www.gstatic.com/firebasejs/12.19.0/firebase-app.js";
import {
    collection,
    deleteDoc,
    doc,
    getDoc,
    getDocs,
    getFirestore,
    setDoc
} from "https://www.gstatic.com/firebasejs/12.19.0/firebase-firestore.js";

const firebaseConfig = {
    apiKey: "AIzaSyCKfynCCzaFig8i74ePzBdsROTY6f-JbIM",
    authDomain: "neuromotion-c343e.firebaseapp.com",
    projectId: "neuromotion-c343e",
    storageBucket: "neuromotion-c343e.firebasestorage.app",
    messagingSenderId: "946353561733",
    appId: "1:946353561733:web:7826de30af10b2633b49ec"
};

const db = window.db || getFirestore(initializeApp(firebaseConfig));
window.db = db;
window.firebaseReady = Promise.resolve({
    db,
    collection,
    deleteDoc,
    doc,
    getDoc,
    getDocs,
    setDoc
});
