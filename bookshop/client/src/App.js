import './App.css';
import { Routes, Route } from "react-router-dom";
import MainPage from "./pages/MainPage/MainPage";
import LoginPage from "./pages/LoginPage";
import { Container } from '@mui/material';
import CartProvider from "./pages/CartPage/CartContext";
import CartPage from "./pages/CartPage/CartPage";

function App() {
  return (
    <div className="App">
      <div className="content">
        <CartProvider>
          <Container>
            <Routes>
              <Route path="/" element={<MainPage />} />
              <Route path="/login?" element={<LoginPage />} />
              <Route path="/basket" element={<CartPage />} />
            </Routes>
          </Container>
        </CartProvider>
      </div>
    </div>
  );
}

export default App;
