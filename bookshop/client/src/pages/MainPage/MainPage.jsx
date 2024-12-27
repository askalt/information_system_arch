import React, { useState } from "react";
import {
    AppBar,
    Toolbar,
    Typography,
    IconButton,
    Grid,
    Badge,
    Container,
    ThemeProvider,
    createTheme,
    Box
} from "@mui/material";
import { FaShoppingCart, FaUserCircle } from "react-icons/fa";
import BookCard from "./BookCard"
import { Link } from 'react-router-dom';

const products = [
    {
        id: 1,
        name: "Оптимизирющие компиляторы",
        image: "https://cdn.litres.ru/pub/c/cover_415/71185981.webp",
        price: 299.99,
        author: "Константин Владимиров"
    },
    {
        id: 2,
        name: "Олимпиадное программирование",
        image: "https://cdn.litres.ru/pub/c/cover_415/44867813.webp",
        price: 199.99,
        author: "Антти Лааксонен"
    },
    {
        id: 3,
        name: "Граф Монте-Кристо. В 2 книгах. Книга 2",
        image: "https://cdn.litres.ru/pub/c/cover_415/68341772.webp",
        price: 1299.99,
        author: "Александр Дюма"
    },
    {
        id: 4,
        name: "Витя Малеев в школе и дома",
        image: "https://cdn.litres.ru/pub/c/cover_415/3140845.webp",
        price: 149.99,
        author: "Николай Носов"
    },
    {
        id: 5,
        name: "Баранкин, будь человеком!",
        image: "https://cdn.litres.ru/pub/c/cover_415/146229.webp",
        price: 499.99,
        author: "Валерий Медведев"
    },
    {
        id: 6,
        name: "Убийство в «Восточном экспрессе»",
        image: "https://cdn.litres.ru/pub/c/cover_415/18922333.webp",
        price: 79.99,
        author: "Кристи Агата"
    }
];

const theme = createTheme({
    palette: {
        primary: {
            main: "#1976d2"
        },
        secondary: {
            main: "#f50057"
        }
    }
});

const MainPage = () => {
    const [cartItems, setCartItems] = useState(0);

    const handleAddToCart = () => {
        setCartItems(prevItems => prevItems + 1);
    };

    const handleRemoveToCart = () => {
        setCartItems(prevItems => prevItems - 1);
    };

    return (
        <ThemeProvider theme={theme}>
            <Box sx={{ flexGrow: 1 }}>
                <AppBar position="fixed">
                    <Toolbar>
                        <Typography variant="h6" component="div" sx={{ flexGrow: 1 }}>
                            Bookshop
                        </Typography>

                        <Link to="/basket">
                            <Badge badgeContent={cartItems} color="secondary">
                                <FaShoppingCart size={26} />
                            </Badge>
                        </Link>

                        <IconButton color="inherit" aria-label="profile">
                            <FaUserCircle size={26} />
                        </IconButton>
                    </Toolbar>
                </AppBar>
                <Container maxWidth="lg" sx={{ mt: 10, mb: 4 }}>
                    <Grid container spacing={4}>
                        {products.map((product) => (
                            <Grid item key={product.id} xs={12} sm={6} md={4}>
                                <BookCard product={product} />
                            </Grid>
                        ))}
                    </Grid>
                </Container>
            </Box>
        </ThemeProvider>
    );
};

export default MainPage;
