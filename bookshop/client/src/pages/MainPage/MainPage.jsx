import React, { useState, useEffect } from 'react';
import {
    Grid,
    Container,
    ThemeProvider,
    createTheme,
    CircularProgress,
    Box
} from "@mui/material";
import BookCard from "./BookCard"
//import { getBooks } from '../../mocks/bookService';

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
    const [loading, setLoading] = useState(false);
    const [books, setBooks] = useState([]);

    useEffect(() => {
        setLoading(true);
        fetch(`http://127.0.0.1:8000/getBooks`)
            .then((response) => response.json())
            .then((books) => {
                setBooks(books);
                setLoading(false);
            });
    }, []);

    return (
        <ThemeProvider theme={theme}>
            <Box sx={{ flexGrow: 1 }}>
                {loading ? (
                    <CircularProgress sx={{ mt: 8 }} />
                ) : (
                    <Container maxWidth="lg" sx={{ mt: 10, mb: 4 }}>
                        <Grid container spacing={4}>
                            {books.map((book) => (
                                <Grid item key={book.id} xs={12} sm={6} md={4}>
                                    <BookCard book={book} />
                                </Grid>
                            ))}
                        </Grid>
                    </Container>
                )}
            </Box>
        </ThemeProvider>
    );
};

export default MainPage;