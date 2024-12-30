import './App.css';
import { Routes, Route } from "react-router-dom";
import MainPage from "./pages/MainPage/MainPage";
import { Container } from '@mui/material';
import CartProvider from "./pages/CartPage/CartContext";
import CartPage from "./pages/CartPage/CartPage";

import {
  AppBar,
  Toolbar,
  Button,
  IconButton,
  Badge,
  Box
} from "@mui/material";
import { Link, useNavigate } from 'react-router-dom';
import { FaShoppingCart, FaUserCircle } from "react-icons/fa";
import BookPage from "./pages/BookPage/BookPage"
import LoginPage from "./pages/LoginPage/LoginPage"
import RegisterPage from "./pages/RegisterPage/RegisterPage"
import { AuthProvider } from "./contexts/AuthContext"
import { isTokenExpired } from './AuthFetch';
import ProfilePage from './pages/ProfilePage/ProfilePage';
//import { useCart } from './pages/CartPage/CartContext';

function App() {
  //const { cartItems } = useCart();
  const navigate = useNavigate();

  const handleProfileClick = (e) => {
    const refreshToken = localStorage.getItem('refresh_token');
    if (!refreshToken || isTokenExpired(refreshToken))
      navigate("/login");
    else
      navigate("/profile");
  };

  return (
    <div className="App">
      <div className="content">
        <AuthProvider>
          <CartProvider>
            <AppBar position="sticky">
              <Toolbar sx={{ display: 'flex', justifyContent: 'space-between' }}>
                <Button color="inherit" component={Link} to="/">
                  Каталог
                </Button>
                <Box sx={{ display: 'flex', gap: 2 }}>
                  <IconButton component={Link} to="/cart" color="inherit" aria-label="cart">
                    <Badge badgeContent={0} color="secondary">
                      <FaShoppingCart size={26} />
                    </Badge>
                  </IconButton>
                  <IconButton onClick={handleProfileClick} color="inherit" aria-label="profile">
                    <FaUserCircle size={26} />
                  </IconButton>
                </Box>
              </Toolbar>
            </AppBar>
            <Container sx={{ mt: 0 }}>
              <Routes>
                <Route path="/" element={<MainPage />} />
                <Route path="/profile" element={<ProfilePage />} />
                <Route path="/login" element={<LoginPage />} />
                <Route path="/register" element={<RegisterPage />} />
                <Route path="/cart" element={<CartPage />} />
                <Route path="/book/:id" element={<BookPage />} />
              </Routes>
            </Container>
          </CartProvider>
        </AuthProvider>
      </div>
    </div>
  );
}

export default App;
