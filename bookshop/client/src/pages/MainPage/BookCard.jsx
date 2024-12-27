import React from "react";
import { Card, CardContent, CardMedia, Typography, Button } from "@mui/material";
import { styled } from "@mui/system";
import { BsCart2 } from "react-icons/bs";
import { useCart } from "../CartPage/CartContext"

const StyledCard = styled(Card)(({ theme }) => ({
    maxWidth: 345,
    margin: "16px",
    transition: "transform 0.3s ease-in-out",
    "&:hover": {
        transform: "translateY(-5px)",
        boxShadow: "0 8px 16px rgba(0,0,0,0.2)"
    }
}));

const StyledCardMedia = styled(CardMedia)({
    height: 400,
    objectFit: "cover"
});

const PriceTypography = styled(Typography)({
    fontWeight: 600,
    color: "#2e7d32",
    marginTop: "8px"
});

const AddToCartButton = styled(Button)(({ theme }) => ({
    marginTop: "16px",
    width: "100%",
    backgroundColor: "#1976d2",
    "&:hover": {
        backgroundColor: "#1565c0"
    }
}));

const BookCard = ({ product }) => {
    const { addToCart } = useCart();

    const handleAddToCart = () => {
        addToCart(product);
    };

    return (
        <StyledCard>
            <StyledCardMedia
                component="img"
                image={product.image}
                alt={product.name}
                onError={(e) => {
                    e.target.src = "https://images.unsplash.com/photo-1505740420928-5e560c06d30e";
                }}
            />
            <CardContent>
                <Typography variant="h6" component="div" gutterBottom>
                    {product.name}
                </Typography>
                <Typography variant="body2" color="text.secondary">
                    {product.author}
                </Typography>
                <PriceTypography variant="h6">
                    {product.price}₽
                </PriceTypography>
                <AddToCartButton
                    variant="contained"
                    startIcon={<BsCart2 />}
                    onClick={handleAddToCart}
                >
                    Add to Cart
                </AddToCartButton>
            </CardContent>
        </StyledCard>
    );
};

export default BookCard;