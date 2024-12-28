import React from "react";
import { Box, Card, CardContent, Typography, Button } from "@mui/material";
import { styled } from "@mui/system";
import { BsCart2 } from "react-icons/bs";
import { useCart } from "../CartPage/CartContext";
import { useNavigate } from 'react-router-dom';
import PriceTypography from '../../components/PriceTypography'

const StyledCard = styled(Card)(({ theme }) => ({
    maxWidth: 345,
    margin: "16px",
    transition: "transform 0.3s ease-in-out",
    "&:hover": {
        transform: "translateY(-5px)",
        boxShadow: "0 8px 16px rgba(0,0,0,0.2)"
    }
}));

const AddToCartButton = styled(Button)(({ theme }) => ({
    marginTop: "16px",
    width: "100%",
    backgroundColor: "#1976d2",
    "&:hover": {
        backgroundColor: "#1565c0"
    }
}));

const BookCard = ({ book }) => {
    const { addToCart } = useCart();
    const navigate = useNavigate();

    const handleOpenBookPage = () => {
        navigate(`/book/${book.id}`);
    };

    const handleAddToCart = (e) => {
        e.stopPropagation();
        addToCart(book);
    };

    return (
        <StyledCard onClick={handleOpenBookPage}>
            <Box sx={{ overflow: 'hidden', width: '100%', height: '400px', position: 'relative' }}>
                <img
                    src={book.image}
                    alt={book.name}
                    style={{
                        width: '100%',
                        height: '100%',
                        objectFit: 'contain',
                        backgroundColor: '#f0f0f0',
                    }}
                />
            </Box>
            <CardContent>
                <Typography variant="h6" component="div" gutterBottom>
                    {book.name}
                </Typography>
                <Typography variant="body2" color="text.secondary">
                    {book.author}
                </Typography>
                <PriceTypography variant="h6">
                    {book.price}₽
                </PriceTypography>

                <AddToCartButton
                    variant="contained"
                    startIcon={<BsCart2 />}
                    onClick={handleAddToCart}
                >
                    Добавить в корзину
                </AddToCartButton>
            </CardContent>
        </StyledCard>
    );
};

export default BookCard;